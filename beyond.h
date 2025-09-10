/* beyond.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Beyond Communication Layer (BEYOND).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef BEYOND_H
#define BEYOND_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define BEYOND_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
#define BEYOND_INLINE __inline__
#elif defined(_MSC_VER)
#define BEYOND_INLINE __inline
#else
#define BEYOND_INLINE
#endif

#define BEYOND_API static

typedef int beyond_bool;

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* #############################################################################
 * # BEYOND Type Checks
 * #############################################################################
 *
 * Verify that the type sizes used in beyond are the same across platforms.
 */
#define BEYOND_STATIC_ASSERT(c, m) typedef char beyond_assert_##m[(c) ? 1 : -1]
BEYOND_STATIC_ASSERT(sizeof(unsigned char) == 1, unsigned_char_size_must_be_1);
BEYOND_STATIC_ASSERT(sizeof(unsigned short) == 2, unsigned_short_size_must_be_2);
BEYOND_STATIC_ASSERT(sizeof(unsigned int) == 4, unsigned_int_size_must_be_4);
BEYOND_STATIC_ASSERT(sizeof(beyond_bool) == 4, int_size_must_be_4);
BEYOND_STATIC_ASSERT(sizeof(float) == 4, float_size_must_be_4);
BEYOND_STATIC_ASSERT(sizeof(double) == 8, double_size_must_be_8);

/* #############################################################################
 * # BEYOND Builtin Memory Functions
 * #############################################################################
 */
BEYOND_API BEYOND_INLINE void *beyond_memset(void *s, int c, unsigned int n)
{
  unsigned char *p = (unsigned char *)s;

  while (n--)
  {
    *p++ = (unsigned char)c;
  }

  return s;
}

BEYOND_API BEYOND_INLINE void *beyond_memcpy(void *dest, void *src, unsigned int count)
{
  char *dest8 = (char *)dest;
  char *src8 = (char *)src;

  while (count--)
  {
    *dest8++ = *src8++;
  }

  return dest;
}

BEYOND_API BEYOND_INLINE void *beyond_memmove(void *dest, void *src, unsigned int n)
{
  unsigned char *d = (unsigned char *)dest;
  unsigned char *s = (unsigned char *)src;

  if (d == s)
  {
    return dest;
  }

  if (d < s)
  {
    /* Copy forward */
    while (n--)
    {
      *d++ = *s++;
    }
  }
  else
  {
    /* Copy backward to handle overlap */
    d += n;
    s += n;
    while (n--)
    {
      *(--d) = *(--s);
    }
  }

  return dest;
}

/* #############################################################################
 * # BEYOND Common Station & Satellite API
 * #############################################################################
 */
#define BEYOND_NULL ((void *)0)

typedef void *beyond_handle;
typedef void *beyond_context;
typedef beyond_bool (*beyond_function_transmit)(beyond_handle handle, beyond_context context, void *payload, unsigned int payload_capacity, unsigned int *payload_size);
typedef beyond_bool (*beyond_function_receive)(beyond_handle handle, beyond_context context, void *payload, unsigned int payload_capacity, unsigned int payload_size);

typedef enum beyond_communication_type
{
  BEYOND_COMMUNICATION_TYPE_UDP = 0

} beyond_communication_type;

typedef struct beyond_address
{
  char *ip;
  int port;

} beyond_address;

BEYOND_API BEYOND_INLINE beyond_address beyond_address_init(char *ip, int port)
{
  beyond_address result;
  result.ip = ip;
  result.port = port;

  return result;
}

/* #############################################################################
 * # BEYOND Satellite API
 * #############################################################################
 */
typedef struct beyond_satellite_api
{

  beyond_communication_type type;
  int transmit_rate_hz;
  beyond_function_transmit transmit;
  beyond_function_receive receive;
  beyond_address address;
  beyond_context context;
  char *transmit_payload;
  unsigned int transmit_payload_capacity;
  char *receive_payload;
  unsigned int receive_payload_capacity;

} beyond_satellite_api;

/* #############################################################################
 * # BEYOND Satellite Platform API
 * #############################################################################
 * 
 * These function have to be implemented by the platform layer (e.g. win32_, linux_, ...)
 */
beyond_handle beyond_satellite_deploy(beyond_satellite_api *api);
beyond_bool beyond_satellite_destroy(beyond_handle satellite);
void beyond_satellite_lock(beyond_handle satellite);
void beyond_satellite_unlock(beyond_handle satellite);
void beyond_satellite_sleep(unsigned long milliseconds);
void beyond_satellite_printf(char *format, ...);

#endif /* BEYOND_H */

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
