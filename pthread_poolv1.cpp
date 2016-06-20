#include<"pthread_poolv1.h">
#include<stdlib>

using namespace std;

int ThreadPool::startUp(){
	int err;
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cond, NULL);
	for(auto aThread: threads){
		if(err=pthread_create(aThread, NULL, runPthread, NULL) != 0)
			ERROEDIE("pthread pool create threads error", -1);
	}
	return 0;
}

int ThreadPool::runThread(){
	int err;
	while(!isCancelled){
		pthread_mutex_lock(&mtx);
		if(!tasks.size())
			if(err=pthread_cond_wait(&cond, &mtx)!=0)
				ERROR_DIE("pthread_cond_wait error",-1);
		if(isCancelled==1){
			pthread_mutex_unlock(&mtx);
			if(err=pthread_cond_broadcast(&cond)!=0)
				ERROR_DIE("pthread_cond_wait error", -1);
			MSG_PRINT("Thread end!");
			return 0;
		}
		ThreadAbstractClass task=tasks.back();
		tasks.pop_back();
		pthread_mutex_unlock(&mtx);
		tasks.run();
	}
	return 0;
}

int ThreadPool::enqueue(ThreadAbstractClass aTask){
	int err;
	if(isWaited==1){
		MSG_PRINT("Thread pool is in waited state, can not enqueue new task");
		return -1;
	}
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
	isWaited=1;
	int siz=0;
	while(1){
		pthread_mutex_lock(&mtx);
		siz=tasks.size();
		pthread_mutex_unlock(&mtx);
		if(siz!=0)
			sleep(1);
		else
			return 0;
	}
}

int ThreadPool::restart(){
	if(isCancelled==1){
		MSGPRINT("Thread pool is being end, can not restart!")	
		return -1;
	}
	isWaited=1;
	MSG_PRINT("Thread pool can be enqueue now!");
	return 0;	
}

int ThreadPool::endPool(){
	int err;
	waitPool();
	isCancelled=1;
	if(err=pthread_cond_broadcast(&cond) !=0 )
		ERROR_DIE("pthread_cond_broadcast error", -1);//isCancelled 为1，此时broadcast将引起连锁broadcast，使得T陆续返回
	pthread_mutex_destory(&mtx);
	pthread_cond_destory(&cond);
	for(auto aThread::threads)
		pthread_join(&aThread, NULL);//默认T的终止状态会保存到调用join
	return 0;		
}
