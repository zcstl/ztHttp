#include"pthread_poolv1.h"
#include<iostream>
#include<unistd.h>
using namespace std;


int ThreadPool::startUp(){
	int err;
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cond, NULL);
	for(auto aThread: threads){//类成员函数，声明为静态，定义是是否要重复指明static？
		if(err=pthread_create(&aThread, NULL, &ThreadPool::runThread, this) != 0)//&funcName,  g++使用成员函数地址需指明作用域,   &thread
			ERROR_DIE("pthread pool create threads error", -1);
	}
	return 0;
}

//void*参数
void* ThreadPool::runThread(void* arg){
	//this=(ThreadPool*)arg;在非静态成员函数里不能使用this关键字
	MSG_PRINT("Thread init!");
	ThreadPool* This=(ThreadPool*)arg;
	int err;
	while(!This->isCancelled){
		pthread_mutex_lock(&This->mtx);
		if(!This->tasks.size())
			if(err=pthread_cond_wait(&This->cond, &This->mtx)!=0)
				ERROR_DIE("pthread_cond_wait error",-1);
		if(This->isCancelled==1){
			pthread_mutex_unlock(&This->mtx);
			if(err=pthread_cond_broadcast(&This->cond)!=0)
				ERROR_DIE("pthread_cond_wait error", -1);
			MSG_PRINT("Thread end!");
			return 0;
		}
		ThreadAbstractClass* task=This->tasks.back();
		//ThreadAbstractClass* task=tmp;
		This->tasks.pop_back();
		pthread_mutex_unlock(&This->mtx);
		task->run();//task为抽象类类型，其run为纯虚函数，没有vtable; 把抽象类的run改为普通虚函数并提供实现，此处编译通过，why
	}
	return (void*)0;
}

void* ThreadAbstractClass::run(){
	return (void*)0;
}

int ThreadPool::enqueue(ThreadAbstractClass* aTask){
	int err;
	if(isWaited==1){
		MSG_PRINT("Thread pool is in waited state, can not enqueue new task");
		return -1;
	}
	MSG_PRINT("Enqueue now!");
	pthread_mutex_lock(&mtx);
	tasks.push_back(aTask);
	pthread_mutex_unlock(&mtx);
	if(err=pthread_cond_broadcast(&cond)!=0)
		ERROR_DIE("enqueue pthread_cond_broadcast error", -1);
	return 0;
}
/**
	当不再接收新task时调用该函数,调用该函数后，等待队列为空，阻塞调用T, 此时：tasks入队操作将返回错误码
**/
int ThreadPool::waitPool(){
    MSG_PRINT("Wait now!!")
	isWaited=1;
	int siz=0;
	while(1){
		pthread_mutex_lock(&mtx);
		siz=tasks.size();
		pthread_mutex_unlock(&mtx);
		MSG_PRINT(siz);
		if(siz!=0)
			sleep(1000);//ms
		else
			return 0;
	}
}

int ThreadPool::reStart(){
	if(isCancelled==1){
		MSG_PRINT("Thread pool is being end, can not restart!")
		return -1;
	}
	isWaited=0;
	MSG_PRINT("Thread pool can be enqueue now!");
	return 0;
}

int ThreadPool::endPool(){
	int err;
	waitPool();
	isCancelled=1;
	if(err=pthread_cond_broadcast(&cond) !=0 )
		ERROR_DIE("pthread_cond_broadcast error", -1);//isCancelled 为1，此时broadcast将引起连锁broadcast，使得T陆续返回
	for(auto aThread:threads);//一个：而不是两个，::是域作用符
		//pthread_join(&aThread, NULL);//默认T的终止状态会保存到调用join
	sleep(5000);
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond);
	return 0;
}
