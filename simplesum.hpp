#ifndef SIMPLESUM_HPP
#define SIMPLESUM_HPP

// Simple arithmetic sum, inverted

class simpleSum {
public:
	typedef unsigned int hashType;
	static void compute (const char *buf, size_t size, unsigned int *sum) {
		unsigned int s = 0;
		const unsigned char *p = (const unsigned char *)buf;
		
		for (size_t i = 0; i < size; i++)
			s += p[i];
		*sum = ~s;
	}
};

#endif

