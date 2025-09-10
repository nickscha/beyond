/* win32_beyond_satellite.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Win32 Beyond Satellite Platform API implementation.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "beyond.h"
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
#define MEM_COMMIT 0x00001000
#define MEM_RELEASE 0x8000
#define MEM_RESERVE 0x00002000
#define PAGE_READWRITE 0x04
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

/* C89-compatible version of LARGE_INTEGER. We use the HighPart and LowPart */
typedef struct LARGE_INTEGER
{
    unsigned long LowPart;
    long HighPart;

} LARGE_INTEGER;

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
WIN32_BEYOND_SATELLITE_API(unsigned long)
WaitForSingleObject(void *hHandle, unsigned long dwMilliseconds);
WIN32_BEYOND_SATELLITE_API(int)
CloseHandle(void *hObject);
WIN32_BEYOND_SATELLITE_API(int)
QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
WIN32_BEYOND_SATELLITE_API(int)
QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
WIN32_BEYOND_SATELLITE_API(void *)
VirtualAlloc(void *lpAddress, unsigned long dwSize, unsigned long flAllocationType, unsigned longflProtect);
WIN32_BEYOND_SATELLITE_API(int)
VirtualFree(void *lpAddress, unsigned long dwSize, unsigned long dwFreeType);
WIN32_BEYOND_SATELLITE_API(void *)
GetStdHandle(unsigned long nStdHandle);
WIN32_BEYOND_SATELLITE_API(int)
WriteConsoleA(void *hConsoleOutput, void *lpBuffer, unsigned longnNumberOfCharsToWrite, unsigned long *lpNumberOfCharsWritten, void *lpReserved);
#else
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
#endif /* _WINDOWS_ */

BEYOND_API BEYOND_INLINE unsigned int win32_strlen(char *str)
{
    char *s = str;

    while (*s)
    {
        s++;
    }

    return (unsigned int)(s - str);
}

void beyond_satellite_printf(char *format, ...)
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
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, win32_strlen(buffer), 0, 0);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), (void *)"\r\n", 2, 0, 0);
    va_end(args);
}

/* This is the concrete implementation of the opaque beyond_satellite handle. */
typedef struct beyond_satellite_handle_internal
{
    /* Internal Platform specific handles */
    unsigned long socket;
    CRITICAL_SECTION lock;
    sockaddr_in server_addr;
    void *transmit_thread_handle;
    void *receive_thread_handle;

    /* The user api defined values */
    beyond_satellite_api *api;

} beyond_satellite_handle_internal;

void beyond_satellite_sleep(unsigned long milliseconds)
{
    Sleep(milliseconds);
}

void beyond_satellite_lock(beyond_handle satellite)
{
    if (satellite)
    {
        EnterCriticalSection(&((beyond_satellite_handle_internal *)satellite)->lock);
    }
}

void beyond_satellite_unlock(beyond_handle satellite)
{
    if (satellite)
    {
        LeaveCriticalSection(&((beyond_satellite_handle_internal *)satellite)->lock);
    }
}

BEYOND_API BEYOND_INLINE unsigned long beyond_satellite_transmit_thread(void *lpParam)
{
    beyond_handle handle = (beyond_handle)lpParam;
    beyond_satellite_handle_internal *satellite = (beyond_satellite_handle_internal *)handle;
    beyond_satellite_api *api = satellite->api;
    int transmit_rate_hz = api->transmit_rate_hz;
    double time_per_tick_ms = 1000.0 / (double)transmit_rate_hz;

    LARGE_INTEGER perf_freq;
    LARGE_INTEGER last_perf_time;
    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&last_perf_time);

    while (1)
    {
        double elapsed_ms;
        LARGE_INTEGER current_perf_time;
        QueryPerformanceCounter(&current_perf_time);

        /* Calculate elapsed time in milliseconds using C89-compatible double arithmetic */
        elapsed_ms = ((double)current_perf_time.HighPart * 4294967296.0 + (double)current_perf_time.LowPart) -
                     ((double)last_perf_time.HighPart * 4294967296.0 + (double)last_perf_time.LowPart);

        elapsed_ms = elapsed_ms * 1000.0 / ((double)perf_freq.HighPart * 4294967296.0 + (double)perf_freq.LowPart);

        if (elapsed_ms >= time_per_tick_ms)
        {
            /* Call the user's handler to fill the buffer */
            unsigned int payload_size = 0;
            beyond_bool transmitted = api->transmit(handle, api->context, api->transmit_payload, api->transmit_payload_capacity, &payload_size);

            if (transmitted && payload_size > 0)
            {
                sendto(satellite->socket, api->transmit_payload, (int)payload_size, 0, (sockaddr *)&satellite->server_addr, sizeof(satellite->server_addr));
            }

            last_perf_time = current_perf_time;
        }

        Sleep(1);
    }
    return 0;
}

BEYOND_API BEYOND_INLINE unsigned long beyond_satellite_receive_thread(void *lpParam)
{
    beyond_handle handle = (beyond_handle)lpParam;
    beyond_satellite_handle_internal *satellite = (beyond_satellite_handle_internal *)handle;
    beyond_satellite_api *api = satellite->api;

    unsigned long timeout = 100; /* 100 milliseoncds */
    sockaddr_in address_from;
    int address_from_length = sizeof(sockaddr_in);

    setsockopt(satellite->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    while (1)
    {

        int bytes_received = recvfrom(satellite->socket, api->receive_payload, (int)api->receive_payload_capacity, 0, (sockaddr *)&address_from, &address_from_length);

        if (bytes_received > 0)
        {
            api->receive(handle, api->context, api->receive_payload, api->receive_payload_capacity, (unsigned int)bytes_received);
        }
        else if (bytes_received == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT)
            {
                beyond_satellite_printf("[satellite] Receiver thread: No packets received in a while...");
            }
        }
    }

    return 0;
}

beyond_handle beyond_satellite_deploy(beyond_satellite_api *api)
{
    WSADATA wsaData;
    beyond_satellite_handle_internal *satellite = VirtualAlloc(0, sizeof(beyond_satellite_handle_internal), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!satellite)
    {
        return BEYOND_NULL;
    }

    if (!api)
    {
        beyond_satellite_printf("[satellite][win32] no valid api entry passed!");
        VirtualFree(satellite, 0, MEM_RELEASE);
        return BEYOND_NULL;
    }

    satellite->api = api;

    /* Initialize Winsock */
    if (WSAStartup(0x0202, &wsaData) != 0)
    {
        beyond_satellite_printf("[satellite] WSAStartup failed.");
        VirtualFree(satellite, 0, MEM_RELEASE);
        return BEYOND_NULL;
    }

    /* Create the UDP socket */
    satellite->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (satellite->socket == INVALID_SOCKET)
    {
        beyond_satellite_printf("[satellite] Socket creation failed.");
        WSACleanup();
        VirtualFree(satellite, 0, MEM_RELEASE);
        return BEYOND_NULL;
    }

    satellite->server_addr.sin_family = AF_INET;
    satellite->server_addr.sin_port = htons((unsigned short)satellite->api->address.port);
    inet_pton(AF_INET, satellite->api->address.ip, &satellite->server_addr.sin_addr);

    InitializeCriticalSection(&satellite->lock);

    if (satellite->api->transmit)
    {
        satellite->transmit_thread_handle = CreateThread(BEYOND_NULL, 0, beyond_satellite_transmit_thread, satellite, 0, BEYOND_NULL);
        beyond_satellite_printf("[beyond][satellite] deployed: transmitting to station at %s:%d", satellite->api->address.ip, satellite->api->address.port);
    }

    if (satellite->api->receive)
    {
        satellite->receive_thread_handle = CreateThread(BEYOND_NULL, 0, beyond_satellite_receive_thread, satellite, 0, BEYOND_NULL);
        beyond_satellite_printf("[beyond][satellite] deployed: receiving from station at %s:%d", satellite->api->address.ip, satellite->api->address.port);
    }

    return satellite;
}

beyond_bool beyond_satellite_destroy(beyond_handle handle)
{
    beyond_satellite_handle_internal *satellite = (beyond_satellite_handle_internal *)handle;

    if (satellite->transmit_thread_handle)
    {
        WaitForSingleObject(satellite->transmit_thread_handle, INFINITE);
        CloseHandle(satellite->transmit_thread_handle);
    }

    if (satellite->receive_thread_handle)
    {
        WaitForSingleObject(satellite->receive_thread_handle, INFINITE);
        CloseHandle(satellite->receive_thread_handle);
    }

    closesocket(satellite->socket);
    WSACleanup();
    DeleteCriticalSection(&satellite->lock);
    VirtualFree(satellite, 0, MEM_RELEASE);

    return true;
}

/* #############################################################################
 * # For Windows DLL builds we have to include at least DllMainCRTStartup
 * #############################################################################
 */
#if defined(_WIN32) || defined(_WIN64)
#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
#ifdef __i686__
__attribute((force_align_arg_pointer))
#endif
int
DllMainCRTStartup(void)
{
    return 1;
}
#endif

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
