# mthasher
Demo project for recruiters, acting on behalf of Veeam software http://veeam.com.
As the recruiters disappeared with no trace, here's the entire solution, as supplied, publicly available on GitHub under the BSD license.

Boost API is used to ensure maximum portability across the operating systems.

Type 'make' to build.
Type 'make check' to test.

Tested under FreeBSD, RedHat Linux, Mac OS X.
Should compile fine on Win32, provided HAVE_MADVISE is undefined in the Makefile.
Hasher extended: a template class is used as interface to the hashing library.
New hashing libraries are added just by adding apropriate class headers.

NOTE: test file not included, on request, due to email attachment size limitations.
