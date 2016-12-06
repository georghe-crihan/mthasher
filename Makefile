INCLUDES=-I/opt/local/include
LIBS=-L/opt/local/lib

#CFLAGS+=-g3

# Comment out on Win32
DEFINES+=-DHAVE_MADVISE=1


# Uncomment lines below, depending on the hash libraries desired/available.
# NOTE: OpenSSL and BSD LIBMD are mutually exclusive.
# Simple arithmetic sum, inverted
HEADERS+=simplesum.hpp
DEFINES+=-DHAVE_SIMPLESUM=1
# OpenSSL with a host of hash functions like ripemd, shaXXX, mdXXX among others
HEADERS+=openssl.hpp
DEFINES+=-DHAVE_OPENSSL=1
EXTRA_LIBS+=-lssl -lcrypto
# BSD-style MD5 library, available in the base system on some platforms 
#HEADERS+=bsd_md5.hpp
#DEFINES+=-DHAVE_BSD_LIBMD=1
# or as libmd on others, both, however, have to be defined in the latter case.
#DEFINES+=-DHAVE_LIBMD=1
#EXTRA_LIBS+=-lmd

all: mthasher

mthasher: mthasher.cpp threadpool.hpp hashif.hpp $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(DEFINES) -o mthasher mthasher.cpp -lboost_program_options-mt -lboost_thread-mt -lboost_iostreams-mt -lboost_system-mt $(EXTRA_LIBS) -lstdc++

check: mthasher
	./mthasher -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	./mthasher -a ripemd160 -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	./mthasher -a md4 -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	./mthasher -a md5 -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	./mthasher -a bsd_md5 -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	./mthasher -a bsd_md2 -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	./mthasher -a bsd_sha -i random_junk -o random_junk.crc32
	-diff -u random_junk.crc32.ref random_junk.crc32
	
stat:
	wc -l *.cpp *.hpp Makefile

src-package: clean
	(cd ..; zip -9 mthasher.zip mthasher/*)

clean:
	rm -f mthasher
	rm -f random_junk.crc32

