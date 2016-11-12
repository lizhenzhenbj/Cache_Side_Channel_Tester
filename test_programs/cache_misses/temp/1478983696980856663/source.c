/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <openssl/rc4.h>
//#include <stdlib.h>
//#include "rc4_locl.h" - start
/*
 * Copyright 1998-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_RC4_LOCL_H
# define HEADER_RC4_LOCL_H

# include <openssl/opensslconf.h>
// # include <internal/cryptlib.h>

#endif
//#include "rc4_locl.h" - end

/*-
 * RC4 as implemented from a posting from
 * Newsgroups: sci.crypt
 * From: sterndark@netcom.com (David Sterndark)
 * Subject: RC4 Algorithm revealed.
 * Message-ID: <sternCvKL4B.Hyy@netcom.com>
 * Date: Wed, 14 Sep 1994 06:35:31 GMT
 */

static unsigned char data[7][30] = {
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0xff},
    {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
     0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
     0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
     0x12, 0x34, 0x56, 0x78, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
    {0},
};

void RC4(RC4_KEY *key, size_t len, const unsigned char *indata,
         unsigned char *outdata)
{
    register RC4_INT *d;
    register RC4_INT x, y, tx, ty;
    size_t i;

    x = key->x;
    y = key->y;
    d = key->data;

#define LOOP(in,out) \n                x=((x+1)&0xff); \n                tx=d[x]; \n                y=(tx+y)&0xff; \n                d[x]=ty=d[y]; \n                d[y]=tx; \n                (out) = d[(tx+ty)&0xff]^ (in);

    i = len >> 3;
    if (i) {
        for (;;) {
            LOOP(indata[0], outdata[0]);
            LOOP(indata[1], outdata[1]);
            LOOP(indata[2], outdata[2]);
            LOOP(indata[3], outdata[3]);
            LOOP(indata[4], outdata[4]);
            LOOP(indata[5], outdata[5]);
            LOOP(indata[6], outdata[6]);
            LOOP(indata[7], outdata[7]);
            indata += 8;
            outdata += 8;
            if (--i == 0)
                break;
        }
    }
    i = len & 0x07;
    if (i) {
        for (;;) {
            LOOP(indata[0], outdata[0]);
            if (--i == 0)
                break;
            LOOP(indata[1], outdata[1]);
            if (--i == 0)
                break;
            LOOP(indata[2], outdata[2]);
            if (--i == 0)
                break;
            LOOP(indata[3], outdata[3]);
            if (--i == 0)
                break;
            LOOP(indata[4], outdata[4]);
            if (--i == 0)
                break;
            LOOP(indata[5], outdata[5]);
            if (--i == 0)
                break;
            LOOP(indata[6], outdata[6]);
            if (--i == 0)
                break;
        }
    }
    key->x = x;
    key->y = y;
}

/* sudiptac: copied functionality from the respective file */
void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data)
{
    register RC4_INT tmp;
    register int id1, id2;
    register RC4_INT *d;
    unsigned int i,j;

		//printf("RC4: %d\nn", sizeof(RC4_INT));

    d = &(key->data[0]);
    key->x = 0;
    key->y = 0;
    id1 = id2 = 0;

#define SK_LOOP(d,n) { \n                tmp=d[(n)]; \n                id2 = (data[id1] + tmp + id2) & 0xff; \n                if (++id1 == len) id1=0; \n                d[(n)]=d[id2]; \n                d[id2]=tmp; }

    for (i = 0; i < 256; i++)
        d[i] = i;

		for (i = 0; i < 16; i += 4) {
    	SK_LOOP(d, i + 0);
   		SK_LOOP(d, i + 1);
    	SK_LOOP(d, i + 2);
    	SK_LOOP(d, i + 3);
		}

    for (j = i; j < 256; j += 4) {
        SK_LOOP(d, j + 0);
        SK_LOOP(d, j + 1);
        SK_LOOP(d, j + 2);
        SK_LOOP(d, j + 3);
    }

}

/* sudiptac: main function for testing RC4 */
int main()
{
    int j;
    unsigned char *p;
    RC4_KEY key;
    unsigned char keys[10] = {5,5,5,5,5,5,5,5,5,5};
    unsigned char obuf[512] = {0};

    long a_fd, m_fd;
    long long accesses, misses;


    /*initializing and starting perf*/
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));

    /*group*/
    pe.type = PERF_TYPE_HW_CACHE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    a_fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (a_fd < 0)
        return -1;
    
    /*child*/
    pe.disabled = 0;
    pe.config = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
    m_fd = syscall(__NR_perf_event_open, &pe, 0, -1, a_fd, 0);

    /*enable and start perf*/
    ioctl(a_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(a_fd, PERF_EVENT_IOC_ENABLE, 0);

    /*start of section - activity to measure*/    
    RC4_set_key(&key, 8, &(keys[1]));
    RC4(&key, 8, &(data[0][0]), obuf);
    /*end of section - activity to measure*/

    /*stopping perf*/
    ioctl(m_fd, PERF_EVENT_IOC_DISABLE, 0);
    
    /*reading outputs and printing them*/
    read(m_fd, &misses, sizeof(long long));
    read(a_fd, &accesses, sizeof(long long));
    printf("%lld\n", accesses);
    printf("%lld\n", misses);

    /*cleanup and exit*/
    close(m_fd);
    close(a_fd);
    return 0;
}