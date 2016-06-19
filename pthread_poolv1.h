#include<pthread.h>
#include<vector>
#include<list>
#ifndef _PTHREAD_POOL_V1_
using namespace std;
typedef void*(*)(void) func;
typedef void* PoolMsg
class ThreadPool{
	public:
		ThreadPool:threads(threadsNumber)(int threadsNumber){}
		//ThreadPool:threads(threadsNumber), isFree(isFree)(int threadsNumber, bool isFree){}
		~ThreadPool(){}
		int startUp();//����pool���߳������ȴ���Ϣ
		int endPool(bool);//�ȴ�����ִ�е�Tִ�н�����ʣ����Ϣ���ٴ����ر��̳߳ص��߳�
		int waitPool();//�����ٽ�������ʱ���øú��������������߳�ֱ����Ϣ���ж���������
		int enqueue(PoolMsg msg, func operation, bool isFree);//	��Ϣ������ӣ���ָ����Ϣ����Ϣ������
	private:
		struct DataNode{
			PoolMsg msg;
			func afunc;
			bool isFree;

		};
		vector<thread_t> threads;
		list<DataNode> nodes;
		/*
		list<PoolMsg> msgs;//
		list<func> funcs;
		list<bool> isFree;
		*/
		pthread_mutex_t mtx;
		pthread_cond_t cond;
		bool isCancelled;//��Ӧ�öԸ�poolֻ��һ��T�б�ʹ��������裬�����T�и�pool������ʱ��һ��T����wait����һ��Ӧ�ÿ���ͨ������end��wait��ǰ����������������ֳ�����ƴ���
};

#endif
