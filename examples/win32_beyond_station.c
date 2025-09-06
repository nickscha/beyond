#include "../beyond.h"
#include "win32_printf.h"

#ifndef _WINDOWS_

#define INVALID_SOCKET (unsigned long)(~0)
#define SOCKET_ERROR (-1)
#define AF_INET 2
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define INADDR_ANY (unsigned long)0x00000000
#define FIONBIO (long)0x8004667E

typedef struct WSADATA
{
    unsigned short wVersion;
    unsigned short wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;

} WSADATA, *LPWSADATA;

/* C89-compatible version of LARGE_INTEGER. We use the HighPart and LowPart */
typedef struct LARGE_INTEGER
{
    unsigned long LowPart;
    long HighPart;

} LARGE_INTEGER;

typedef struct in_addr
{
    unsigned long s_addr;
} in_addr;

typedef struct sockaddr
{
    unsigned short sa_family;
    char sa_data[14];
} sockaddr;

typedef struct sockaddr_in
{
    short sin_family;
    unsigned short sin_port;
    in_addr sin_addr;
    char sin_zero[8];
} sockaddr_in;

#define WIN32_BEYOND_STATION_API(r) __declspec(dllimport) r __stdcall

WIN32_BEYOND_STATION_API(int)
WSAStartup(unsigned short wVersionRequested, LPWSADATA lpWSAData);
WIN32_BEYOND_STATION_API(int)
WSACleanup(void);
WIN32_BEYOND_STATION_API(unsigned long)
socket(int af, int type, int protocol);
WIN32_BEYOND_STATION_API(int)
bind(unsigned long s, sockaddr *name, int namelen);
WIN32_BEYOND_STATION_API(int)
closesocket(unsigned long s);
WIN32_BEYOND_STATION_API(unsigned short)
htons(unsigned short hostshort);
WIN32_BEYOND_STATION_API(unsigned long)
inet_addr(char *cp);
WIN32_BEYOND_STATION_API(int)
ioctlsocket(unsigned long s, long cmd, unsigned long *argp);
WIN32_BEYOND_STATION_API(int)
sendto(unsigned long s, char *buf, int len, int flags, sockaddr *to, int tolen);
WIN32_BEYOND_STATION_API(int)
recvfrom(unsigned long s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
WIN32_BEYOND_STATION_API(int)
QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
WIN32_BEYOND_STATION_API(int)
QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
#else
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
#endif /* _WINDOWS_ */

/* The packet the client sends to the server, containing its input. */
typedef struct beyond_packet_user_input
{
    long tick;
    float x;
    float y;
} beyond_packet_user_input;

/* A single user's state within the server's world. */
typedef struct beyond_packet_server_state_user
{
    int user_id;
    float x;
    float y;
} beyond_packet_server_state_user;

/* The packet the server sends back, containing all user states. */
#define MAX_USERS 100
typedef struct beyond_packet_server_state
{
    long tick;
    long last_processed_tick;
    int user_count;
    beyond_packet_server_state_user users[MAX_USERS];
} beyond_packet_server_state;

/* An entry for each client connected to the server. */
typedef struct beyond_packet_client_entry
{
    int user_id;
    sockaddr_in addr;
    long last_processed_tick;
    long last_received_tick;
    beyond_packet_user_input last_input;

} beyond_packet_client_entry;

#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
#ifdef __i686__
__attribute((force_align_arg_pointer))
#endif
int
mainCRTStartup(void)
{
    WSADATA wsaData;
    unsigned long server_socket;
    sockaddr_in server_addr;
    beyond_packet_client_entry clients[MAX_USERS];
    int client_count = 0;
    int i;
    long current_server_tick = 0;
    unsigned long TICK_RATE_MS = 10; /* 100 Hz */
    long CLIENT_TIMEOUT_TICKS = 300; /* Client is removed after 3 seconds of inactivity */
    unsigned long non_blocking = 1;
    LARGE_INTEGER perf_freq;
    LARGE_INTEGER last_perf_time;

    win32_printf("[station] startup");

    beyond_memset(clients, 0, sizeof(clients));

    if (WSAStartup(0x0202, &wsaData) != 0)
    {
        win32_printf("[station] socket creation failed.");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (server_socket == INVALID_SOCKET)
    {
        win32_printf("[station] socket failed.");
        WSACleanup();
        return 1;
    }
    beyond_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        win32_printf("[station] binding failed.");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    /* Set socket to non-blocking */
    ioctlsocket(server_socket, FIONBIO, &non_blocking);

    /* Get performance counter frequency for high-resolution timing */
    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&last_perf_time);

    win32_printf("[station] started and listening on signal 8080");

    while (1)
    {
        LARGE_INTEGER current_perf_time;
        double elapsed_ms;
        QueryPerformanceCounter(&current_perf_time);

        /* Calculate elapsed time in milliseconds using C89-compatible double arithmetic */
        elapsed_ms = ((double)current_perf_time.HighPart * 4294967296.0 + (double)current_perf_time.LowPart) -
                     ((double)last_perf_time.HighPart * 4294967296.0 + (double)last_perf_time.LowPart);
        elapsed_ms = elapsed_ms * 1000.0 / ((double)perf_freq.HighPart * 4294967296.0 + (double)perf_freq.LowPart);

        if (elapsed_ms >= TICK_RATE_MS)
        {
            /* Process all pending client inputs */
            beyond_packet_user_input input_packet;
            sockaddr_in from_addr;
            beyond_packet_server_state state_packet;
            int from_addr_len = sizeof(from_addr);
            int bytes_received;

            last_perf_time = current_perf_time;
            current_server_tick++;

            while ((bytes_received = recvfrom(server_socket, (char *)&input_packet, sizeof(input_packet), 0,
                                              (sockaddr *)&from_addr, &from_addr_len)) > 0)
            {
                if (bytes_received == sizeof(beyond_packet_user_input))
                {
                    /* Find the client in our list, or add a new one */
                    int client_index = -1;
                    for (i = 0; i < client_count; ++i)
                    {
                        if (clients[i].addr.sin_addr.s_addr == from_addr.sin_addr.s_addr &&
                            clients[i].addr.sin_port == from_addr.sin_port)
                        {
                            client_index = i;
                            break;
                        }
                    }

                    if (client_index == -1 && client_count < MAX_USERS)
                    {
                        client_index = client_count++;
                        clients[client_index].user_id = client_index; /* Assign a new ID */
                        clients[client_index].addr = from_addr;
                        win32_printf("[station] new satellite connected, ID: %d", clients[client_index].user_id);
                    }

                    if (client_index != -1)
                    {
                        clients[client_index].last_input = input_packet;
                        clients[client_index].last_processed_tick = input_packet.tick;
                        clients[client_index].last_received_tick = current_server_tick; /* Update last received tick */
                        win32_printf("[station] Received input from satellite %d at tick %d. Pos: %.3lf, %.3lf", clients[client_index].user_id, input_packet.tick, (double)input_packet.x, (double)input_packet.y);
                    }
                }
            }

            /* Timeout check for idle clients (iterate backwards to safely remove) */
            i = 0;
            while (i < client_count)
            {
                if (current_server_tick - clients[i].last_received_tick > CLIENT_TIMEOUT_TICKS)
                {
                    win32_printf("[station] satellite %d timed out and was removed.", clients[i].user_id);
                    clients[i] = clients[client_count - 1];
                    client_count--;
                }
                else
                {
                    i++;
                }
            }

            /* Game Logic Update (Fixed-tick) */
            for (i = 0; i < client_count; ++i)
            {
                clients[i].last_input.x += 1.0f;
                clients[i].last_input.y += 0.5f;
            }

            /* Broadcast the game state to all clients */
            state_packet.tick = current_server_tick;
            state_packet.user_count = client_count;

            for (i = 0; i < client_count; ++i)
            {
                state_packet.users[i].user_id = clients[i].user_id;
                state_packet.users[i].x = clients[i].last_input.x;
                state_packet.users[i].y = clients[i].last_input.y;
            }

            for (i = 0; i < client_count; ++i)
            {
                state_packet.last_processed_tick = clients[i].last_processed_tick;
                sendto(server_socket, (char *)&state_packet, sizeof(state_packet), 0, (sockaddr *)&clients[i].addr, sizeof(sockaddr_in));
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
