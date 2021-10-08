#ifndef MD5_H_
#define MD5_H_

typedef guint32 md5_size;

struct md5_ctx
{
	struct
	{
		guint32 A, B, C, D;
	} regs;
    guchar* buf;
    md5_size size;
    md5_size bits;
};

#define MD5_BUFFER 1024

#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (~z & y))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | ~z))

#define ROTATE_LEFT(w,s) ((w << s) | ((w & 0xFFFFFFFF) >> (32 - s)))

#define FF(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + F(b,c,d) + x + t), s))
#define GG(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + G(b,c,d) + x + t), s))
#define HH(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + H(b,c,d) + x + t), s))
#define II(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + I(b,c,d) + x + t), s))

guchar* md5(guchar*, md5_size, guchar*);
void md5_init (struct md5_ctx*);
void md5_update (struct md5_ctx* context);
void md5_final (guchar* digest, struct md5_ctx* context);

#endif /* MD5_H_ */
