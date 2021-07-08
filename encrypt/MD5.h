//md5.h头文件的内容开始

#pragma once

typedef BYTE * POINTER;
typedef unsigned short int UINT2;
typedef unsigned long int UINT4;
#define PROTO_LIST(list) list

//MD5 context.
typedef struct _MD5_CTX {
	UINT4 state[4];                                   // state (ABCD)
	UINT4 count[2];        // number of bits, modulo 2^64 (lsb first)
	BYTE buffer[64];                         // input buffer
} _MD5_CTX;


void MD5Init PROTO_LIST ((_MD5_CTX *));
void MD5Update PROTO_LIST
((_MD5_CTX *, BYTE *, unsigned int));
void MD5Final PROTO_LIST ((BYTE [16], _MD5_CTX *));

/* Constants for MD5Transform routine.
*/


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

static void MD5Transform PROTO_LIST ((UINT4 [4], BYTE [64]));
static void Encode PROTO_LIST
((BYTE *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
((UINT4 *, BYTE *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static BYTE PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}


class CMD5
{
private:
	void MD5Update (_MD5_CTX * context, BYTE * input, unsigned int inputLen);
	void MD5Final (BYTE digest[16], _MD5_CTX * context);
	static void MD5Transform (UINT4 state[4], BYTE block[64]);
	void MD5ToHex(BYTE * Out,BYTE * lpbuffer,DWORD dwSize);
public:
	CMD5(void);
	~CMD5(void);
	void MDString (BYTE * string,BYTE * out,int len);
};

//运行所需参数
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000
static void MDString PROTO_LIST ((CHAR *));
static void MDTimeTrial PROTO_LIST ((void));
static void MDTestSuite PROTO_LIST ((void));
static void MDFile PROTO_LIST ((CHAR *));
static void MDFilter PROTO_LIST ((void));
static void MDPrint PROTO_LIST ((BYTE [16]));
#define _MD5_CTX _MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final
/* Digests a string and prints the result.
*/

//md5.h文件结束
