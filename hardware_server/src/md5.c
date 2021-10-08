#include <glib.h>
#include <assert.h>
#include "md5.h"
#include <glib/gstdio.h>

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define memcopy(a,b,c) md5_memcopy ((a), (b), (c))
#define memset(a,b,c) md5_memset ((a), (b), (c))

#define GET_UINT32(a,b,i)                     \
{                                             \
         (a) = ( (guint32) (b)[(i)  ]      )  \
             | ( (guint32) (b)[(i)+1] << 8 )  \
             | ( (guint32) (b)[(i)+2] << 16)  \
             | ( (guint32) (b)[(i)+3] << 24); \
}

static void md5_memcopy (guchar* dst, guchar* src, const guint32 length);
static void md5_memset (guchar*, const guchar symbol, const guint32 length);
static void md5_addsize (guchar*, md5_size , md5_size);
static void md5_encode (guchar*, struct md5_ctx *);

static guchar MD5_PADDING[64] =
{
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

guchar* md5(guchar* M, md5_size len, guchar* _digest)
{
	gint32 buflen = (len > MD5_BUFFER) ? MD5_BUFFER: len;
    struct md5_ctx* context;

    context = malloc (sizeof (struct md5_ctx));
    context->buf = malloc (buflen);
    context->size = 0;
    context->bits = 0;

    context->regs.A = 0x67452301;
    context->regs.B = 0xefcdab89;
    context->regs.C = 0x98badcfe;
    context->regs.D = 0x10325476;

    do
    {
    	memcopy (context->buf + context->size, M + context->bits, buflen - context->size);
        context->size += buflen - context->size;
        md5_update (context);
    } while (len - context->bits > 64);

    md5_final(_digest, context);

    free(context->buf);
    free(context);

    return _digest;
}

void md5_init(struct md5_ctx *context)
{
	context->buf = malloc (MD5_BUFFER);
    memset (context->buf, '\0', MD5_BUFFER);
    context->size = 0;
    context->bits = 0;

    context->regs.A = 0x67452301;
    context->regs.B = 0xefcdab89;
    context->regs.C = 0x98badcfe;
    context->regs.D = 0x10325476;
}

static void md5_addsize (guchar* M, md5_size index, md5_size oldlen)
{
	assert (((index * 8) % 512) == 448);

    M[index++] = (guchar) ((oldlen << 3) & 0xFF);
    M[index++] = (guchar) ((oldlen >> 5) & 0xFF);
    M[index++] = (guchar) ((oldlen >> 13) & 0xFF);
    M[index++] = (guchar) ((oldlen >> 21) & 0xFF);

    M[index++] = 0; M[index++] = 0;
    M[index++] = 0; M[index++] = 0;
}

void md5_update(struct md5_ctx* context)
{
	guchar buffer[64];

	gint i = 0;

	for (i = 0; context->size - i > 63; i += 64)
	{
		memcopy(buffer, context->buf + i, 64);
        md5_encode(buffer, context);
        context->bits += 64;
    }

	memcopy (buffer, context->buf + i, context->size - i);
    memcopy (context->buf, buffer, context->size - i);
    context->size -= i;
}

void md5_final (unsigned char *digest, struct md5_ctx *context)
{
    guchar buffer[64] = {0x00};
    gint i;

    assert (context->size < 64);

    if (context->size + 1 > 56)
    {
    	memcopy(buffer, context->buf, context->size);
        memcopy(buffer + context->size, MD5_PADDING, 64 - context->size);
        md5_encode(buffer, context);
        context->bits += context->size;
        context->size = 0;

        memset(buffer, '\0', 56);
        md5_addsize(buffer, 56, context->bits);
        md5_encode (buffer, context);
    }
    else
    {
    	memcopy (buffer, context->buf, context->size);
    	context->bits += context->size;

        memcopy (buffer + context->size, MD5_PADDING, 56 - context->size);
        md5_addsize (buffer, 56, context->bits);
        md5_encode (buffer, context);
    }

    for (i = 0; i < 4; i++)
    	digest [i] = (guchar) ((context->regs.A >> (i*8)) & 0xFF);
    for (; i < 8; i++)
        digest [i] = (guchar) ((context->regs.B >> ((i-4)*8)) & 0xFF);
    for (; i < 12; i++)
        digest [i] = (guchar) ((context->regs.C >> ((i-8)*8)) & 0xFF);
    for (; i < 16; i++)
        digest [i] = (guchar) ((context->regs.D >> ((i-12)*8)) & 0xFF);
}

static void md5_encode (guchar* buffer, struct md5_ctx* context)
{
	guint32 a = context->regs.A, b = context->regs.B, c = context->regs.C, d = context->regs.D;
    guint x[16];

    GET_UINT32 (x[ 0],buffer, 0);
    GET_UINT32 (x[ 1],buffer, 4);
    GET_UINT32 (x[ 2],buffer, 8);
    GET_UINT32 (x[ 3],buffer,12);
    GET_UINT32 (x[ 4],buffer,16);
    GET_UINT32 (x[ 5],buffer,20);
    GET_UINT32 (x[ 6],buffer,24);
    GET_UINT32 (x[ 7],buffer,28);
    GET_UINT32 (x[ 8],buffer,32);
    GET_UINT32 (x[ 9],buffer,36);
    GET_UINT32 (x[10],buffer,40);
    GET_UINT32 (x[11],buffer,44);
    GET_UINT32 (x[12],buffer,48);
    GET_UINT32 (x[13],buffer,52);
    GET_UINT32 (x[14],buffer,56);
    GET_UINT32 (x[15],buffer,60);

    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */

    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    context->regs.A += a;
    context->regs.B += b;
    context->regs.C += c;
    context->regs.D += d;
}

static void md5_memcopy(guchar* dest, guchar* src, guint32 length)
{
	for (guint32 i = 0; i < length; i++)
	{
		dest [i] = src [i];
    }
}

static void md5_memset(guchar* dst, const guchar symbol, const guint32 length)
{
	for (guint32 i = 0; i < length; i++)
	{
         dst[i] = symbol;
    }
}
