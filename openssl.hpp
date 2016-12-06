#ifndef OPENSSL_H
#define OPENSSL_H

#include <stdio.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/md4.h>
#include <openssl/md5.h>

template<class T>
class openssl {
public:
	typedef char hashType[T::DIGEST_SIZE*2+1];
	static void compute(const char *buf, size_t size, hashType *sum) {
		unsigned char digest[T::DIGEST_SIZE];

		T::DigestFn((const unsigned char *)buf, size, digest);
		
		for (size_t i = 0; i < T::DIGEST_SIZE; i++)
			sprintf(&((*sum)[i*2]), "%02X", digest[i]);  
		(*sum)[T::DIGEST_SIZE*2] = '\0';
	}
};

class sha256 {
public:
	static const size_t DIGEST_SIZE = SHA256_DIGEST_LENGTH;
	static void DigestFn(const unsigned char *buf, size_t size, unsigned char *digest) {
		SHA256(buf, size, digest);
	}
};

class ripemd160 {
public:
	static const size_t DIGEST_SIZE = RIPEMD160_DIGEST_LENGTH;
	static void DigestFn(const unsigned char *buf, size_t size, unsigned char *digest) {
		RIPEMD160(buf, size, digest);
	}
};

class md4 {
public:
	static const size_t DIGEST_SIZE = MD4_DIGEST_LENGTH;
	static void DigestFn(const unsigned char *buf, size_t size, unsigned char *digest) {
		MD4(buf, size, digest);
	}
};

class md5 {
public:
	static const size_t DIGEST_SIZE = MD5_DIGEST_LENGTH;
	static void DigestFn(const unsigned char *buf, size_t size, unsigned char *digest) {
		MD5(buf, size, digest);
	}
};

#endif

