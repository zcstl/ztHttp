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
		int startUp();//����pool���߳������ȴ���Ϣ
		int endPool(bool);//�ȴ�����ִ�е�Tִ�н�����ʣ����Ϣ���ٴ����ر��̳߳ص��߳�
		int waitPool();//�����ٽ�������ʱ���øú��������������߳�ֱ����Ϣ���ж���������
		int enqueue(PoolMsg msg, func operation);//	��Ϣ������ӣ���ָ����Ϣ����Ϣ������
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
