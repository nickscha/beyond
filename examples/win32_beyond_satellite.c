#include "../beyond.h"

#include <stdarg.h>

#ifndef _WINDOWS_

#define INFINITE 0xffffffff
#define WSABASEERR 10000
#define SOL_SOCKET 0xffff
#define SO_RCVTIMEO 0x1006
#define WSAETIMEDOUT (WSABASEERR + 60)
#define INVALID_SOCKET (unsigned long)(~0)
#define SOCKET_ERROR (-1)
#define AF_INET 2
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
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

typedef struct CRITICAL_SECTION
{
    void *DebugInfo;
    long LockCount;
    long RecursionCount;
    void *OwningThread;
    void *LockSemaphore;
    unsigned long SpinCount;
} CRITICAL_SECTION;

typedef unsigned long (*PTHREAD_START_ROUTINE)(void *lpThreadParameter);

#define WIN32_BEYOND_SATELLITE_API(r) __declspec(dllimport) r __stdcall

WIN32_BEYOND_SATELLITE_API(int)
WSAStartup(unsigned short wVersionRequested, LPWSADATA lpWSAData);
WIN32_BEYOND_SATELLITE_API(int)
WSACleanup(void);
WIN32_BEYOND_SATELLITE_API(int)
WSAGetLastError(void);
WIN32_BEYOND_SATELLITE_API(unsigned long)
socket(int af, int type, int protocol);
WIN32_BEYOND_SATELLITE_API(int)
setsockopt(unsigned long s, int level, int optname, char *optval, int optlen);
WIN32_BEYOND_SATELLITE_API(int)
closesocket(unsigned long s);
WIN32_BEYOND_SATELLITE_API(unsigned short)
htons(unsigned short hostshort);
WIN32_BEYOND_SATELLITE_API(int)
sendto(unsigned long s, char *buf, int len, int flags, sockaddr *to, int tolen);
WIN32_BEYOND_SATELLITE_API(int)
recvfrom(unsigned long s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
WIN32_BEYOND_SATELLITE_API(void *)
GetStdHandle(unsigned long nStdHandle);
WIN32_BEYOND_SATELLITE_API(int)
WriteConsoleA(void *hConsoleOutput, void *lpBuffer, unsigned longnNumberOfCharsToWrite, unsigned long *lpNumberOfCharsWritten, void *lpReserved);
WIN32_BEYOND_SATELLITE_API(void)
Sleep(unsigned long dwMilliseconds);
WIN32_BEYOND_SATELLITE_API(void)
EnterCriticalSection(void *lpCriticalSection);
WIN32_BEYOND_SATELLITE_API(void)
LeaveCriticalSection(void *lpCriticalSection);
WIN32_BEYOND_SATELLITE_API(void)
InitializeCriticalSection(void *lpCriticalSection);
WIN32_BEYOND_SATELLITE_API(void)
DeleteCriticalSection(void *lpCriticalSection);
WIN32_BEYOND_SATELLITE_API(int)
inet_pton(int Family, char *pStringBuf, void *pAddr);
WIN32_BEYOND_SATELLITE_API(void *)
CreateThread(void *lpThreadAttributes, unsigned long dwStackSize, PTHREAD_START_ROUTINE lpStartAddress, void *lpParameter, unsigned long dwCreationFlags, unsigned long *lpThreadId);
#else
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
#endif /* _WINDOWS_ */

/* The packet the client sends to the server, containing its input. */
typedef struct USER_INPUT_PACKET
{
    long tick;
    float x;
    float y;

} USER_INPUT_PACKET;

/* A single user's state within the server's world. */
typedef struct USER_STATE
{
    int user_id;
    float x;
    float y;

} USER_STATE;

/* The packet the server sends back, containing all user states. */
#define MAX_USERS 100
typedef struct SATELLITE_STATE_PACKET
{
    long tick;
    long last_processed_tick;
    int user_count;
    USER_STATE users[MAX_USERS];

} SATELLITE_STATE_PACKET;

/* A context for the threads, holding shared resources */
#define INPUT_HISTORY_SIZE 600 /* Store up to 10 seconds of history at 60 FPS */
typedef struct THREAD_CONTEXT
{
    unsigned long socket;
    sockaddr_in server_addr;
    CRITICAL_SECTION pos_lock;
    float x_pos;
    float y_pos;
    long last_sent_tick;
    USER_INPUT_PACKET input_history[INPUT_HISTORY_SIZE];
    long history_tail;

} THREAD_CONTEXT, *PTHREAD_CONTEXT;

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

BEYOND_API BEYOND_INLINE unsigned long beyond_satellite_send(void *lpParam)
{
    PTHREAD_CONTEXT context = (PTHREAD_CONTEXT)lpParam;
    unsigned long client_socket = context->socket;
    sockaddr_in server_addr = context->server_addr;
    USER_INPUT_PACKET input_packet;
    int server_addr_len = sizeof(server_addr);

    beyond_printf("[satellite] communication with station established");

    while (1)
    {
        /* 1. Prepare the input packet */
        EnterCriticalSection(&context->pos_lock);
        context->last_sent_tick++;
        input_packet.x = context->x_pos;
        input_packet.y = context->y_pos;
        input_packet.tick = context->last_sent_tick;
        LeaveCriticalSection(&context->pos_lock);

        /* 2. Store the packet in the history buffer */
        context->input_history[context->history_tail] = input_packet;
        context->history_tail = (context->history_tail + 1) % INPUT_HISTORY_SIZE;

        /* 3. Send the packet to the server */
        sendto(client_socket, (char *)&input_packet, sizeof(input_packet), 0, (sockaddr *)&server_addr, server_addr_len);

        /* 4. Simulate movement (this is the "client-side prediction") */
        EnterCriticalSection(&context->pos_lock);
        context->x_pos += 1.0f;
        context->y_pos += 0.5f;
        LeaveCriticalSection(&context->pos_lock);

        Sleep(10); /* Simulates sending at a 100 Hz rate */
    }
    return 0;
}

BEYOND_API BEYOND_INLINE unsigned long beyond_satellite_receive(void *lpParam)
{
    PTHREAD_CONTEXT context = (PTHREAD_CONTEXT)lpParam;
    unsigned long client_socket = context->socket;
    SATELLITE_STATE_PACKET received_packet;
    int bytes_received;
    int from_addr_len;
    struct sockaddr_in from_addr;
    int i;
    unsigned long timeout = 1000; /* 1-second timeout */

    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    beyond_printf("[satellite] receiver started");

    while (1)
    {
        from_addr_len = sizeof(from_addr);

        bytes_received = recvfrom(client_socket, (char *)&received_packet, sizeof(received_packet), 0,
                                  (sockaddr *)&from_addr, &from_addr_len);

        if (bytes_received > 0 && bytes_received == sizeof(SATELLITE_STATE_PACKET))
        {
            beyond_printf("[satellite] Received State Update at Station Tick: %d (Last Processed: %d)", received_packet.tick, received_packet.last_processed_tick);

            /* Lag Compensation Logic */
            /* 1. Find our own position update in the server's packet */
            for (i = 0; i < received_packet.user_count; i++)
            {
                if (received_packet.users[i].user_id == 0) /* Our ID is 0 for this example */
                {
                    /* 2. We've found our user state. Now, find the corresponding input packet in our history.  */
                    long server_processed_tick = received_packet.last_processed_tick;
                    long history_index = (context->history_tail - (context->last_sent_tick - server_processed_tick) - 1 + INPUT_HISTORY_SIZE) % INPUT_HISTORY_SIZE;

                    /* 3. Reconcile our state. */
                    EnterCriticalSection(&context->pos_lock);

                    /* Correct our position to match the server's "ground truth" */
                    context->x_pos = received_packet.users[i].x;
                    context->y_pos = received_packet.users[i].y;

                    /* Re-apply inputs from the server-processed tick to the present */
                    while (history_index != context->history_tail)
                    {
                        context->x_pos += 1.0f;
                        context->y_pos += 0.5f;
                        history_index = (history_index + 1) % INPUT_HISTORY_SIZE;
                    }

                    LeaveCriticalSection(&context->pos_lock);

                    beyond_printf("[satellite]  - Reconciled Position to x:%.3lf, y:%.3lf", (double)context->x_pos, (double)context->y_pos);
                    break;
                }
            }

            /* Print positions of all users */
            for (i = 0; i < received_packet.user_count; i++)
            {
                beyond_printf("[satellite]  - User %d at x:%.3lf, y:%.3lf", received_packet.users[i].user_id, (double)received_packet.users[i].x, (double)received_packet.users[i].y);
            }
        }
        else if (bytes_received == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT)
            {
                beyond_printf("[satellite] Receiver thread: No packets received in a while...");
            }
        }
    }
    return 0;
}

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
    unsigned long client_socket;
    struct sockaddr_in server_addr;
    THREAD_CONTEXT context;

    beyond_printf("[satellite] startup");

    if (WSAStartup(0x0202, &wsaData) != 0)
    {
        beyond_printf("[satellite] socket initialization failed");
        return 1;
    }
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket == INVALID_SOCKET)
    {
        beyond_printf("[satellite] socket failed.");
        WSACleanup();
        return 1;
    }

    beyond_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(8080);
    context.socket = client_socket;
    context.server_addr = server_addr;
    InitializeCriticalSection(&context.pos_lock);
    context.x_pos = 0.0;
    context.y_pos = 0.0;
    context.last_sent_tick = 0;
    context.history_tail = 0;

    CreateThread((void *)0, 0, beyond_satellite_send, &context, 0, (void *)0);
    CreateThread((void *)0, 0, beyond_satellite_receive, &context, 0, (void *)0);
    Sleep(INFINITE);

    closesocket(client_socket);
    WSACleanup();
    DeleteCriticalSection(&context.pos_lock);

    return 0;
}
