#include "pthread_poolv1.h"
#include <iostream>
#include <unistd.h>
class MockThread:public ThreadAbstractClass{
	public:
		MockThread(int i):a(i){}
		void* run(){
			int b=1;
			while(b<=10)
				cout<<a<<" "<<++b<<endl, sleep(1);
			return (void*)a;
		}
	private:
		int a;
};

int main(int argc, char* argv[]){
	MockThread t1(1);
	MockThread t2(2);
	MockThread t3(3);
	MockThread t4(4);
	ThreadPool* tp=new ThreadPool(2);
	tp->startUp();
	MSG_PRINT("1\n");
	tp->enqueue(&t1);
	tp->enqueue(&t2);
	tp->enqueue(&t3);
	tp->enqueue(&t4);
	MSG_PRINT("2\n");
	tp->waitPool();
	tp->enqueue(&t1);
	tp->enqueue(&t1);
	tp->reStart();
	MSG_PRINT("3\n");
	tp->enqueue(&t1);
	tp->enqueue(&t2);
	tp->enqueue(&t3);
	tp->enqueue(&t4);
	tp->endPool();
	MSG_PRINT("4\n");
	sleep(5000);
	delete tp;
	return 0;
}
