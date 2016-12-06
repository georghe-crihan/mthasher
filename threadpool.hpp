#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

struct threadPool {
private:
	typedef boost::movelib::unique_ptr<boost::asio::io_service::work> basio_worker;
	boost::asio::io_service ioS;
	basio_worker work;
	boost::thread_group tg;	
	
public:
	threadPool(size_t numThreads) :ioS(), work(new basio_worker::element_type(ioS)) {
		while (numThreads--) {
			tg.add_thread(new boost::thread(boost::bind(&boost::asio::io_service::run, &(this->ioS))));
			
		}
	}
	~threadPool() {
		work.reset();
		tg.join_all();
		ioS.stop();
	}
	
	template<class T>
	void enqueue(T m) {
		ioS.post(m);
	}
};

#endif

