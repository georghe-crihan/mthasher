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
#include <vector>

#include <boost/crc.hpp>

static size_t gChunkSize = 1024 * 1024;
static int gNumThreads = boost::thread::hardware_concurrency();

namespace po = boost::program_options;
namespace bios = boost::iostreams;
namespace basio = boost::asio;

class SignatureArray {
private:
  size_t whole_chunks;
  boost::shared_ptr<unsigned int[]> sa;

public:
  SignatureArray(size_t n, size_t chunkSize) {
    whole_chunks = 0;

    whole_chunks = n / chunkSize;
    if (n % chunkSize) whole_chunks++;
    sa = boost::make_shared<unsigned int[]>(whole_chunks, 0); 
    
  };
  unsigned int *get(const int i) {
    return &sa[i];
  };
  void store(std::ofstream *ofs) {
    for ( size_t i = 0; i < whole_chunks; i++)
      *ofs << std::hex << std::uppercase << sa[i] << std::endl;
  };
};

static void write_signature_file(std::ofstream *ofs, size_t chunkSize, SignatureArray *a)
{
try {
// Uncomment below to have the chunk and file size included as first line of
// signature file, i.e., handling the last odd chunk CRC.
//    *ofs << std::hex << std::uppercase << chunkSize << std::endl;
    a->store(ofs);
} catch (const std::ofstream::failure &f) {
      std::cout << "Writing to outfile failed" << f.what() << "\n";
}
}
 
static void compute_checksum(const char *buf, size_t size, unsigned int *sum)
{
boost::crc_32_type crc;

#ifdef HAVE_MADVISE
  madvise((void *)buf, size, MADV_SEQUENTIAL);
#endif

  crc.process_bytes( buf, size );
  *sum = crc.checksum();
}

int main(int argc, char **argv)
{
bios::mapped_file_source infile;
std::ofstream ofs;
ofs.exceptions ( std::ofstream::failbit | std::ofstream::badbit);

try {
  std::string inf;
  std::string otf;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("chunksize,c", po::value<int>(), "set chunk size")
    ("numthreads,n", po::value<int>(), "set number of threads")
    ("infile,i", po::value(&inf), "the <infile> to create CRC32 from")
    ("outfile,o", po::value(&otf), "the <outfile> to write CRC32 to")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    exit(EXIT_SUCCESS);
  }

  if (vm.count("chunksize")) {
    gChunkSize = vm["chunksize"].as<int>();
    std::cout << "Chunk size of " << gChunkSize << ".\n";
    if (gChunkSize % getpagesize())
      std::cout << "Warning: for best performance, the chunksize should be a multiple of " << getpagesize() << ".\n"; 
  } 

  if (vm.count("numthreads")) {
    gNumThreads = vm["numthreads"].as<int>();
    std::cout << "Number of threads is " << gNumThreads << ".\n";
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
  
} catch(std::exception& e) {
  std::cerr << e.what() << "\n";
}

  boost::thread_group tg;
  basio::io_service ioS;
  SignatureArray a(infile.size(), gChunkSize);

  try {
    typedef boost::scoped_ptr<basio::io_service::work> basio_worker;
    basio_worker work(new basio_worker::element_type(ioS));

    try {
      for (int i = 0; i < gNumThreads; i++)
        tg.create_thread(boost::bind(&basio::io_service::run, &ioS));
     } catch (const std::exception &f) {
       std::cerr << f.what() << "\n";
       exit(EXIT_FAILURE);
     }

    for (int i = 0, j = 0; i < infile.size(); i+=gChunkSize, j++)
      ioS.post(boost::bind(compute_checksum, infile.data() + i, 
      // Handle last chunk
      (i + gChunkSize <= infile.size()) ? gChunkSize : infile.size() - i,
      a.get(j)));

    work.reset();
    tg.join_all();
    ioS.stop();

    write_signature_file(&ofs, gChunkSize, &a);
    ofs.close();
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}

