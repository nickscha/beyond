
#ifndef WIN32_PRINTF_H
#define WIN32_PRINTF_H

#include "../beyond.h"
#include <stdarg.h>

#ifndef _WINDOWS_

#define STD_OUTPUT_HANDLE ((unsigned long)-11)
#define WIN32_API(r) __declspec(dllimport) r __stdcall

WIN32_API(void *)
GetStdHandle(unsigned long nStdHandle);
WIN32_API(int)
WriteConsoleA(void *hConsoleOutput, void *lpBuffer, unsigned longnNumberOfCharsToWrite, unsigned long *lpNumberOfCharsWritten, void *lpReserved);

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

BEYOND_API BEYOND_INLINE void win32_printf(char *format, ...)
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

#endif /* WIN32_PRINTF_H*/
