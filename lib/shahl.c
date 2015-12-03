/* shahl.c
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dkuug.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * $Id: shahl.c,v 1.8.2.1 1998/02/18 02:24:05 jkh Exp $
 *
 */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "sha.h"

char *
SHAEnd(SHA_CTX *ctx, char *buf)
{
    int i;
    unsigned char digest[SHA_HASHBYTES];
    static const char hex[]="0123456789abcdef";

    if (!buf)
        buf = malloc(33);
    if (!buf)
	return 0;
    SHAFinal(digest,ctx);
    for (i=0;i<SHA_HASHBYTES;i++) {
	buf[i+i] = hex[digest[i] >> 4];
	buf[i+i+1] = hex[digest[i] & 0x0f];
    }
    buf[i+i] = '\0';
    return buf;
}

char *
SHAFile (const char *filename, char *buf)
{
    unsigned char buffer[BUFSIZ];
    SHA_CTX ctx;
    int f,i,j;

    SHAInit(&ctx);
    f = open(filename,O_RDONLY);
    if (f < 0) return 0;
    while ((i = read(f,buffer,sizeof buffer)) > 0) {
	SHAUpdate(&ctx,buffer,i);
    }
    j = errno;
    close(f);
    errno = j;
    if (i < 0) return 0;
    return SHAEnd(&ctx, buf);
}

char *
SHAData (const unsigned char *data, unsigned int len, char *buf)
{
    SHA_CTX ctx;

    SHAInit(&ctx);
    SHAUpdate(&ctx,data,len);
    return SHAEnd(&ctx, buf);
}
