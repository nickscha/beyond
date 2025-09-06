#include "../beyond.h"

#include <stdarg.h>

#ifndef _WINDOWS_

#define INVALID_SOCKET (unsigned long)(~0)
#define SOCKET_ERROR (-1)
#define AF_INET 2
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define INADDR_ANY (unsigned long)0x00000000
#define FIONBIO (long)0x8004667E
#define STD_OUTPUT_HANDLE ((unsigned long)-11)

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
WIN32_BEYOND_STATION_API(void *)
GetStdHandle(unsigned long nStdHandle);
WIN32_BEYOND_STATION_API(int)
WriteConsoleA(void *hConsoleOutput, void *lpBuffer, unsigned longnNumberOfCharsToWrite, unsigned long *lpNumberOfCharsWritten, void *lpReserved);
WIN32_BEYOND_STATION_API(unsigned long)
GetTickCount(void);

#endif /* _WINDOWS_ */

BEYOND_API BEYOND_INLINE void beyond_printf(char *format, ...)
{
    char buffer[256];
    char *buf_ptr = buffer;
    char *format_ptr = format;
    va_list args;
    va_start(args, format);

    while (*format_ptr != '\0' && ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1)))
    {
        if (*format_ptr == '%')
        {
            format_ptr++;
            if (*format_ptr == 's')
            {
                char *s = va_arg(args, char *);
                while (*s != '\0' && ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1)))
                {
                    *buf_ptr++ = *s++;
                }
            }
            else if (*format_ptr == 'd')
            {
                int n = va_arg(args, int);
                char num_str[16];
                int i = 0;
                int is_neg = 0;

                if (n < 0)
                {
                    is_neg = 1;
                    n = -n;
                }
                if (n == 0)
                {
                    num_str[i++] = '0';
                }
                else
                {
                    while (n > 0)
                    {
                        num_str[i++] = (char)(n % 10) + '0';
                        n /= 10;
                    }
                }
                if (is_neg)
                {
                    *buf_ptr++ = '-';
                }
                while (i > 0 && ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1)))
                {
                    *buf_ptr++ = num_str[--i];
                }
            }
            else if (*format_ptr == '.' && *(format_ptr + 1) == '3' && *(format_ptr + 2) == 'l' && *(format_ptr + 3) == 'f')
            {
                double d = va_arg(args, double);
                long long_part = (long)d;
                double frac_part = d - long_part;

                char int_str[32];
                int i = 0;
                int j = 0;
                long frac_int;
                char frac_str[4];

                format_ptr += 3;

                if (long_part == 0)
                {
                    int_str[i++] = '0';
                }
                else
                {
                    while (long_part > 0)
                    {
                        int_str[i++] = (char)(long_part % 10) + '0';
                        long_part /= 10;
                    }
                }

                while (j < i && ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1)))
                {
                    *buf_ptr++ = int_str[i - 1 - j];
                    j++;
                }

                if ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1))
                {
                    *buf_ptr++ = '.';
                }

                frac_int = (long)(frac_part * 1000.0);

                if (frac_int < 0)
                {
                    frac_int = -frac_int;
                }

                beyond_memset(frac_str, '0', 3);
                i = 0;
                if (frac_int > 0)
                {
                    do
                    {
                        frac_str[i++] = (char)(frac_int % 10) + '0';
                        frac_int /= 10;
                    } while (frac_int > 0 && i < 3);
                }
                for (j = 2; j >= 0; j--)
                {
                    if ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1))
                    {
                        *buf_ptr++ = frac_str[j];
                    }
                }
            }
        }
        else
        {
            if ((unsigned long)(buf_ptr - buffer) < (unsigned long)(sizeof(buffer) - 1))
            {
                *buf_ptr++ = *format_ptr;
            }
        }
        format_ptr++;
    }
    *buf_ptr = '\0';
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, beyond_strlen(buffer), (void *)0, (void *)0);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "\r\n", 2, (void *)0, (void *)0);
    va_end(args);
}

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

} beyond_packet_client_entry, *Pbeyond_packet_client_entry;

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
    unsigned long last_tick_time = GetTickCount();
    unsigned long TICK_RATE_MS = 10; /* 100 Hz */
    long CLIENT_TIMEOUT_TICKS = 300; /* Client is removed after 3 seconds of inactivity */
    unsigned long non_blocking = 1;

    beyond_printf("[station] startup");

    beyond_memset(clients, 0, sizeof(clients));

    if (WSAStartup(0x0202, &wsaData) != 0)
    {
        beyond_printf("[station] socket creation failed.");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (server_socket == INVALID_SOCKET)
    {
        beyond_printf("[station] socket failed.");
        WSACleanup();
        return 1;
    }
    beyond_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        beyond_printf("[station] binding failed.");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    /* Set socket to non-blocking */
    ioctlsocket(server_socket, FIONBIO, &non_blocking);

    beyond_printf("[station] started and listening on signal 8080");

    while (1)
    {
        unsigned long current_time = GetTickCount();

        if (current_time - last_tick_time >= TICK_RATE_MS)
        {
            /* Process all pending client inputs */
            beyond_packet_user_input input_packet;
            sockaddr_in from_addr;
            beyond_packet_server_state state_packet;
            int from_addr_len = sizeof(from_addr);
            int bytes_received;

            last_tick_time = current_time;
            current_server_tick++;

            while ((bytes_received = recvfrom(server_socket, (char *)&input_packet, sizeof(input_packet), 0,
                                              (struct sockaddr *)&from_addr, &from_addr_len)) > 0)
            {
                if (bytes_received == sizeof(beyond_packet_user_input))
                {
                    /* Find the client in our list, or add a new one */
                    int client_index = -1;
                    for (i = 0; i < client_count; i++)
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
                        beyond_printf("[station] new satellite connected, ID: %d", clients[client_index].user_id);
                    }

                    if (client_index != -1)
                    {
                        clients[client_index].last_input = input_packet;
                        clients[client_index].last_processed_tick = input_packet.tick;
                        clients[client_index].last_received_tick = current_server_tick; /* Update last received tick */
                        beyond_printf("[station] Received input from satellite %d at tick %d. Pos: %.3lf, %.3lf", clients[client_index].user_id, input_packet.tick, (double)input_packet.x, (double)input_packet.y);
                    }
                }
            }

            /* Timeout check for idle clients (iterate backwards to safely remove) */
            for (i = client_count - 1; i >= 0; i--)
            {
                if (current_server_tick - clients[i].last_received_tick > CLIENT_TIMEOUT_TICKS)
                {
                    beyond_printf("[station] satellite %d timed out and was removed.", clients[i].user_id);
                    /* Shift the remaining clients to fill the gap */
                    if (i < client_count - 1)
                    {
                        beyond_memmove(&clients[i], &clients[i + 1], (unsigned int)(client_count - 1 - i) * (unsigned int)sizeof(beyond_packet_client_entry));
                    }
                    client_count--;
                }
            }

            /* Game Logic Update (Fixed-tick) */
            for (i = 0; i < client_count; i++)
            {
                clients[i].last_input.x += 1.0f;
                clients[i].last_input.y += 0.5f;
            }

            /* Broadcast the game state to all clients */
            state_packet.tick = current_server_tick;
            state_packet.user_count = client_count;
            for (i = 0; i < client_count; i++)
            {
                state_packet.users[i].user_id = clients[i].user_id;
                state_packet.users[i].x = clients[i].last_input.x;
                state_packet.users[i].y = clients[i].last_input.y;
            }

            for (i = 0; i < client_count; i++)
            {
                state_packet.last_processed_tick = clients[i].last_processed_tick;
                sendto(server_socket, (char *)&state_packet, sizeof(state_packet), 0,
                       (struct sockaddr *)&clients[i].addr, sizeof(sockaddr_in));
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
