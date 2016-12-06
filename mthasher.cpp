#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/mman.h> /* madvise() */
#include <stdlib.h> /* exit() */
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <memory>

#include <boost/crc.hpp>

#include "threadpool.hpp"
#include "hashif.hpp"

// Hash libraries
#ifdef HAVE_SIMPLESUM
#include "simplesum.hpp"
#endif

#if defined(HAVE_BSD_LIBMD) && defined(HAVE_OPENSSL)
#error OPENSSL and BSD_LIBMD libraries are mutually exclusive.
#endif

#ifdef HAVE_BSD_LIBMD
#include "bsd_md5.hpp"
#endif

#ifdef HAVE_OPENSSL
#include "openssl.hpp"
#endif

#if BOOST_VERSION < 105900
#error The program requires BOOST >= 10.59.00
#endif

namespace mthasher {
	class crc32 { // failsafe, make sure no collision with other CRC32 implementation ever occurs.
	public:
		typedef unsigned int hashType;
		static void compute(const char *buf, size_t size, hashType *sum) {
			boost::crc_32_type crc;

			crc.process_bytes( buf, size );
			*sum = crc.checksum();
		}
	};
}

int main(int argc, char **argv)
{	
try {
	boost::iostreams::mapped_file_source infile;
	std::ofstream ofs;
	ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	
	std::string inf;
	std::string otf;
	std::string algo;

	size_t chunkSize = 1024 * 1024;
	size_t numThreads = boost::thread::hardware_concurrency();

	namespace po = boost::program_options;

	po::options_description desc("Allowed options");
	desc.add_options()
	("help,h", "produce help message")
    ("chunksize,c", po::value<int>(), "set chunk size")
    ("numthreads,n", po::value<int>(), "set number of threads")
    ("infile,i", po::value(&inf), "the <infile> to create CRC32 from")
    ("outfile,o", po::value(&otf), "the <outfile> to write CRC32 to")
	("algorithm,a", po::value(&algo), "the hash algorithm to use, default is CRC32")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << "\n";
		exit(EXIT_SUCCESS);
	}

	if (vm.count("chunksize")) {
		chunkSize = vm["chunksize"].as<int>();
		std::cout << "Chunk size of " << chunkSize << ".\n";
		if (chunkSize % getpagesize())
			std::cout << "Warning: for best performance, the chunksize should be a multiple of " << getpagesize() << ".\n"; 
	} 

	if (vm.count("numthreads")) {
		numThreads = vm["numthreads"].as<int>();
		std::cout << "Number of threads is " << numThreads << ".\n";
	}

	if (vm.count("outfile")) {
		try {
			ofs.open(otf.c_str());
		} catch (const std::ofstream::failure &f) {
			std::cerr << "Exception opening outfile " << f.what() << "\n";
			exit(EXIT_FAILURE);
		}
	} else {
		std::cout << "output file is mandatory!\n";
		exit(EXIT_FAILURE);
	}

  if (vm.count("infile")) {
    try {
      infile.open(inf.c_str());
     } catch (std::ios_base::failure &f) {
       std::cerr<< "Input file mapping failed " << f.what() << "\n";
       exit(EXIT_FAILURE);
     } catch (std::exception &f) {
       std::cerr << "Exception opening infile " << f.what() << "\n";
       exit(EXIT_FAILURE);
     }
  } else {
    std::cout << "input file is mandatory!\n";
    exit(EXIT_FAILURE);
  }
  
	if (vm.count("algorithm")) {
#ifdef HAVE_SIMPLESUM
		if (algo=="simpleSum") {
			boost::scoped_ptr<mthasher::HashInterface<simpleSum> > hi(
				new mthasher::HashInterface<simpleSum>(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
#endif
#ifdef HAVE_BSD_LIBMD
			if (algo=="bsd_sha") {
			boost::scoped_ptr<mthasher::HashInterface<bsd_sha> > hi(
			    new mthasher::HashInterface<bsd_sha>(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
			if (algo=="bsd_md2") {
			boost::scoped_ptr<mthasher::HashInterface<bsd_md2> > hi(
			    new mthasher::HashInterface<bsd_md2>(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
			if (algo=="bsd_md4") {
			boost::scoped_ptr<mthasher::HashInterface<bsd_md4> > hi(
			    new mthasher::HashInterface<bsd_md4>(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
			if (algo=="bsd_md5") {
			boost::scoped_ptr<mthasher::HashInterface<bsd_md5> > hi(
			    new mthasher::HashInterface<bsd_md5>(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
#endif
#ifdef HAVE_OPENSSL
			if (algo=="sha256") {
			boost::scoped_ptr<mthasher::HashInterface<openssl<sha256> > > hi(
			    new mthasher::HashInterface<openssl<sha256> >(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
			if (algo=="ripemd160") {
			boost::scoped_ptr<mthasher::HashInterface<openssl<ripemd160> > > hi(
				new mthasher::HashInterface<openssl<ripemd160> >(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else 
			if (algo=="md4") {
			boost::scoped_ptr<mthasher::HashInterface<openssl<md4> > > hi(
				new mthasher::HashInterface<openssl<md4> >(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
		} else
			if (algo=="md5") {
			boost::scoped_ptr<mthasher::HashInterface<openssl<md5> > > hi(
				new mthasher::HashInterface<openssl<md5> >(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();
			} else
				
#endif
			if (algo=="crc32") {
		default_algo:
			boost::scoped_ptr<mthasher::HashInterface<mthasher::crc32> > hi(
			    new mthasher::HashInterface<mthasher::crc32>(&infile, &ofs, numThreads, chunkSize));
			hi->run();
			hi.reset();			
		} else // default
			goto default_algo;
	} else goto default_algo;

} catch(std::exception& e) {
	std::cerr << e.what() << "\n";
}	
	return 0;
}

