#include<pthread.h>
#include<vector>
#include<list>
#include "ztHttp/ztHttp/printMsg.h""
#ifndef PTHREAD_POOL_V1
#define PTHREAD_POOL_V1

using namespace std;
//typedef void*(*)(void) func;
//typedef void* PoolMsg

class ThreadAbstractClass{
	public:
		//virtual void* run();//(void)应该需要传参数
		virtual void* run();//(void)应该需要传参数
};


class ThreadPoolAbstractClass{
	public:
		virtual int startUp()=0;
		virtual int waitPool()=0;
		virtual int reStart()=0;
		virtual int endPool()=0;
		virtual int enqueue(ThreadAbstractClass*)=0;
};

class ThreadPool:public ThreadPoolAbstractClass{
	public:
		//构造函数的定义是否可以与初始化列表分开，在cpp实现？
		//此处mtx，cond和isCanceled可执行默认初始化，然后在startUp里进行赋值
		ThreadPool(int threadsNumber):isWaited(0), isCancelled(0), threads(threadsNumber){}
		//ThreadPool:threads(threadsNumber), isFree(isFree)(int threadsNumber, bool isFree){}
		~ThreadPool(){}//析构函数的分析，指针，内存释放，此处默认即可
		int startUp();//开启pool，线程阻塞等待消息
		int endPool();//等待正在执行的T执行结束，剩余消息不再处理，关闭线程池的线程
		int waitPool();//当不再接收请求时调用该函数，阻塞调用线程直到消息队列都被处理完
		//int enqueue(PoolMsg msg, func operation, bool isFree);//	消息队列入队，需指明消息和消息处理函数
		int reStart();
		int enqueue(ThreadAbstractClass* athread);
	private:
		/*
		struct DataNode{
			PoolMsg msg;
			func afunc;
			bool isFree;

		};
		*/
		//int runThread();
		static void* runThread(void*);//pthread_create参数要求
		vector<pthread_t> threads;//该类的实现细节
		/*违反DIP*/
		//list<DataNode> nodes;
		list<ThreadAbstractClass*> tasks;
		/*
		list<PoolMsg> msgs;//
		list<func> funcs;
		list<bool> isFree;
		*/
		pthread_mutex_t mtx;
		pthread_cond_t cond;
		bool isWaited;
		bool isCancelled;//不应该对该pool只在一个T中被使用提出假设，当多个T有该pool的引用时，一个T调用wait，另一个应该可以通过调用end是wait提前结束返还，否则出现程序设计错误
};

#endif
