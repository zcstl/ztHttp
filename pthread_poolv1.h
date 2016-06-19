#include<pthread.h>
#include<vector>
#include<list>
#ifndef _PTHREAD_POOL_V1_
using namespace std;
typedef void*(*)(void) func;
class ThreadPool{
	public:
		ThreadPool:threads(threadsNumber)(int threadsNumber){}
		//ThreadPool:threads(threadsNumber), isFree(isFree)(int threadsNumber, bool isFree){}
		~ThreadPool(){}
		int startUp();//开启pool，线程阻塞等待消息
		int endPool(bool);//等待正在执行的T执行结束，剩余消息不再处理，关闭线程池的线程
		int waitPool();//当不再接收请求时调用该函数，阻塞调用线程直到消息队列都被处理完
		int enqueue(PoolMsg msg, func operation);//	消息队列入队，需指明消息和消息处理函数
	private:
		vector<thread_t> threads;
		list<PoolMsg> msgs;
		list<func> funcs;
		pthread_mutex_t mtx;
		pthread_cond_t cond;
		//bool isCancelled
		//bool isFree;
};

#endif
