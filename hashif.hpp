#ifndef HASHIF_HPP
#define HASHIF_HPP

namespace mthasher {
	template<class T>
	class SignatureArray {
	private:
		size_t whole_chunks;
		boost::shared_ptr<typename T::hashType[]> sa;
	
	public:
		SignatureArray(size_t n, size_t chunkSize) {
			whole_chunks = 0;
		
			whole_chunks = n / chunkSize;
			if (n % chunkSize) whole_chunks++;
			sa = boost::make_shared_noinit<typename T::hashType[]>(whole_chunks); 
		}
		typename T::hashType *get(const int i) {
			return &sa[i];
		}
		void store(std::ofstream *ofs) {
			for (size_t i = 0; i < whole_chunks; i++)
				*ofs << std::hex << std::uppercase << sa[i] << std::endl;
		}
	};
	
	template<class T>
	class HashInterface : public SignatureArray<T> {
	private:
		boost::thread_group tg;
		boost::asio::io_service ioS;
		size_t chunkSize;
		size_t numThreads;
		std::ofstream *ofs;
		boost::iostreams::mapped_file_source *infile;
		boost::scoped_ptr<threadPool> tp;
		
	public:
		HashInterface(boost::iostreams::mapped_file_source *infile, std::ofstream *ofs, size_t numThreads, size_t chunkSize) :SignatureArray<T>(infile->size(), chunkSize), tp(new threadPool(numThreads)), chunkSize(chunkSize), numThreads(numThreads), ofs(ofs), infile(infile) {
		};
		~HashInterface() {
			tp.reset();
			writeSignatureFile();
			// Be paranoid!
			ofs->close();
		}
		void run() {			
			try {
				for (int i = 0, j = 0; i < infile->size(); i+=chunkSize, j++)
					tp->enqueue(boost::bind(this->compute, infile->data() + i, 
											// Handle last chunk
											(i + chunkSize <= infile->size()) ? chunkSize : infile->size() - i,
											this->get(j)));
			} catch (std::exception &e) {
				std::cerr << e.what() << "\n";
				exit(EXIT_FAILURE);
			}			
		}
		void writeSignatureFile() {
			try {
				// Uncomment below to have the chunk and file size included as first line of
				// signature file, i.e., handling the last odd chunk CRC.
//				*ofs << std::hex << std::uppercase << chunkSize << std::endl;
				this->store(ofs);
			} catch (const std::ofstream::failure &f) {
				std::cout << "Writing to outfile failed" << f.what() << "\n";
			}
		}
		static void compute(const char *buf, size_t size, typename T::hashType *sum) {
#ifdef HAVE_MADVISE
			madvise((void *)buf, size, MADV_SEQUENTIAL);
#endif
			T::compute(buf, size, sum);
		}
	};
}

#endif
