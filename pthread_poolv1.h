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
		//virtual void* run();//(void)Ӧ����Ҫ������
		virtual void* run();//(void)Ӧ����Ҫ������
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
		//���캯���Ķ����Ƿ�������ʼ���б�ֿ�����cppʵ�֣�
		//�˴�mtx��cond��isCanceled��ִ��Ĭ�ϳ�ʼ����Ȼ����startUp����и�ֵ
		ThreadPool(int threadsNumber):isWaited(0), isCancelled(0), threads(threadsNumber){}
		//ThreadPool:threads(threadsNumber), isFree(isFree)(int threadsNumber, bool isFree){}
		~ThreadPool(){}//���������ķ�����ָ�룬�ڴ��ͷţ��˴�Ĭ�ϼ���
		int startUp();//����pool���߳������ȴ���Ϣ
		int endPool();//�ȴ�����ִ�е�Tִ�н�����ʣ����Ϣ���ٴ����ر��̳߳ص��߳�
		int waitPool();//�����ٽ�������ʱ���øú��������������߳�ֱ����Ϣ���ж���������
		//int enqueue(PoolMsg msg, func operation, bool isFree);//	��Ϣ������ӣ���ָ����Ϣ����Ϣ������
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
		static void* runThread(void*);//pthread_create����Ҫ��
		vector<pthread_t> threads;//�����ʵ��ϸ��
		/*Υ��DIP*/
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
		bool isCancelled;//��Ӧ�öԸ�poolֻ��һ��T�б�ʹ��������裬�����T�и�pool������ʱ��һ��T����wait����һ��Ӧ�ÿ���ͨ������end��wait��ǰ����������������ֳ�����ƴ���
};

#endif
