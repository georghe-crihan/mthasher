#ifndef BSD_MD5_H
#define BSD_MD5_H

// BSD-style MD library.

#include <sys/types.h>
extern "C" {
#ifdef HAVE_LIBMD
#include <libmd/md2.h>
#include <libmd/md4.h>
#include <libmd/md5.h>
#include <libmd/sha.h>
#else
// Included in the base system from BSD v2.0
#include <sha.h>
#include <md2.h>
#include <md4.h>
#include <md5.h>
#endif
}

class bsd_md5 {
public:
	typedef char hashType[33];
	static void compute(const char *buf, size_t size, hashType *sum) {
		MD5Data((const unsigned char *)buf, (unsigned int) size, *sum);
	}
};

class bsd_md2 {
public:
	typedef char hashType[33];
	static void compute(const char *buf, size_t size, hashType *sum) {
		MD2Data((const unsigned char *)buf, (unsigned int) size, *sum);
	}
};

class bsd_md4 {
public:
	typedef char hashType[33];
	static void compute(const char *buf, size_t size, hashType *sum) {
		MD4Data((const unsigned char *)buf, (unsigned int) size, *sum);
	}
};

class bsd_sha {
public:
	typedef char hashType[41];
	static void compute(const char *buf, size_t size, hashType *sum) {
		SHAData((const unsigned char *)buf, (unsigned int) size, *sum);
  }
};

#endif

