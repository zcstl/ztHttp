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
	ThreadPool tp(2);
	tp.startUp();
	tp.enqueue(t1);
	tp.enqueue(t2);
	tp.enqueue(t3);
	tp.enqueue(t4);
	tp.waitPool();
	tp.enqueue(t1);
	tp.enqueue(t1);
	tp.reStart();
	tp.enqueue(t1);
	tp.enqueue(t2);
	tp.enqueue(t3);
	tp.enqueue(t4);
	tp.endPool();
	
	
}
