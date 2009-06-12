/*
 *
 *  Copyright (C) 2007, 2008, Waseda University.
 *  All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <String.h>
#include <Types.h>

int
memcmp(const void *s1, const void *s2, size_t len)
{
    const char *ptr1 = static_cast<const char *>(s1);
    const char *ptr2 = static_cast<const char *>(s2);
    for (size_t i = 0; i < len; i++) {
        if (ptr1[i] < ptr2[i]) {
            return -1;
        }
        if (ptr1[i] > ptr2[i]) {
            return 1;
        }
    }
    return 0;
}

void *
memcpy(void *dest, const void *src, size_t len)
{
    volatile char   *ptr1 = (volatile char *)dest;
    const char      *ptr2 = (const char *)src;

    for (size_t i = 0; i < len; i++) {
        ptr1[i] = ptr2[i];
    }

    return dest;
}

void *
memset(void *dest, int c, size_t len)
{
    volatile char    *ptr = (volatile char *)dest;

    for (size_t i = 0; i < len; i++) {
        ptr[i] = (char)c;
    }

    return dest;
}

size_t
strlen(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++) ;
    return i;
}

int
strncmp(const char *str1, const char *str2, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (str1[i] == '\0' && str2[i] == '\0') {
            return 0;
        }
        if (str1[i] < str2[i]) {
            return -1;
        }
        if (str1[i] > str2[i]) {
            return 1;
        }
    }
    return 0;
}

int
strcmp(const char *str1, const char *str2)
{
    int i = 0;
    while (str1[i] != '\0' || str2[i] != '\0') {
        if (str1[i] < str2[i]) {
            return -1;
        }
        if (str1[i] > str2[i]) {
            return 1;
        }
        i++;
    }
    return 0;
}

char *
strcpy(char *dest, const char *src)
{
    char *dptr = dest;
    const char *sptr = src;

    while (*sptr != '\0') {
        *dptr = *sptr;
        dptr++;
        sptr++;
    }
    *dptr = '\0';
    return dest;
}

char *
strncpy(char *dest, const char *src, size_t n)
{
    for (UInt i = 0; i < n; i++) {
        dest[i] = src[i];
        if (src[i] == '\0') {
            break;
        }
    }
    return dest;
}

