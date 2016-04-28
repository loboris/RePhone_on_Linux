/*	$OpenBSD: sha1.h,v 1.5 2007/09/10 22:19:42 henric Exp $	*/

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#ifndef _SHA1_H_
#define _SHA1_H_

#define	SHA1_BLOCK_LENGTH		64
#define	SHA1_DIGEST_LENGTH		20

typedef struct {
	uint32_t	state[5];
	uint64_t	count;
	unsigned char	buffer[SHA1_BLOCK_LENGTH];
} SHA1_CTX;
  
static void SHA1Init(SHA1_CTX * context);
static void SHA1Transform(uint32_t state[5], const unsigned char buffer[SHA1_BLOCK_LENGTH]);
static void SHA1Update(SHA1_CTX *context, const unsigned char *data, unsigned int len);
static void SHA1Final(unsigned char digest[SHA1_DIGEST_LENGTH], SHA1_CTX *context);

#endif /* _SHA1_H_ */
