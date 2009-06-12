/*
 * Copyright (c) 2004, National ICT Australia
 */
/*
 * Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: Ben Leslie Created: Fri Oct 8 2004 
 */

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

/**
 * Work out the numeric value of a char, assuming up to base 36
 *  
 * @param ch The character to decode
 *
 * \return Numeric value of character, or 37 on failure
 */
static inline unsigned short
char_value(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 10;
    }

    return 37;
}

long int
strtol(const char *nptr, char **endptr, int base)
{
    /*
     * Decompose input info thread parts: - inital list of whitespace (as per
     * isspace) - subject sequence - final string one or more unrecognized 
     */
    const char *ptr = nptr;
    bool negative = false;
    unsigned int value;
    long int return_value = 0;

    /* Remove spaces */
    while (*ptr != '\0') {
        if (!isspace(*ptr)) {
            break;
        }
        ptr++;
    }

    if (*ptr == '\0')
        goto fail;

    /* check [+|-] */
    if (*ptr == '+') {
        ptr++;
    } else if (*ptr == '-') {
        negative = true;
        ptr++;
    }

    if (*ptr == '\0')
        goto fail;

    if (base == 16) {
        /* _May_ have 0x prefix */
        if (*ptr == '0') {
            ptr++;
            if (*ptr == 'x' || *ptr == 'X') {
                ptr++;
            }
        }
    }

    /* [0(x|X)+] */
    if (base == 0) {
        /* Could be hex or octal or decimal */
        if (*ptr != '0') {
            base = 10;
        } else {
            ptr++;
            if (*ptr == '\0')
                goto fail;
            if (*ptr == 'x' || *ptr == 'X') {
                base = 16;
                ptr++;
            } else {
                base = 8;
            }
        }
    }

    if (*ptr == '\0')
        goto fail;

    /* Ok, here we have a base, and we might have a valid number */
    value = char_value(*ptr);
    if (value >= base) {
        goto fail;
    } else {
        return_value = value;
        ptr++;
    }

    while (*ptr != '\0' && (value = char_value(*ptr)) < base) {
        return_value = return_value * base + value;
        ptr++;
    }

    if (endptr != NULL)
        *endptr = (char *)ptr;

    if (negative) {
        return_value *= -1;
    }

    return return_value;

    /* if base is 0, then we work it out based on a couple of things */
    /* [+|-][0(x|X)+][0-9A-Za-z] not LL * */

    /* endptr == final string */

  fail:
    if (endptr != NULL)
        *endptr = (char *)nptr;
    return 0;

}
