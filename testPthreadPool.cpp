#include "pthread_poolv1.h"
#include <iostream>
#include <unistd.h>
/*
class MockThread:public ThreadAbstractClass{
	public:
		MockThread(int i):a(i){}
		void* run(){
			int b=1;
			while(b<=2)
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
	tp->waitPool();//此处没有阻塞
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
	sleep(5);
	delete tp;
	return 0;
}
*/

/*
int main(){
    //static_cast
    const float a=1.1;
    static_cast<const int>(a);
    static_cast<int>(a);
    //const_cast
    const float b=0.1;
    const float* c=&b;
    const_cast<float*>(c);
    const float d=1.1;
    const float & ff=b;
    const_cast<float&>(ff);
    //const_cast<float>(d);//error,d非指针和引用
    //reinterprete
    const float e=1.1;
    const float* f=&e;
    //reinterpret_cast<const int*>(e);
    //reinterpret_cast<int>(f);//error
    //reinterpret_cast<int>(e);
    //reinterpret_cast<int*>(f);
    reinterpret_cast<const float*>(c);
    const float & fff=e;
    reinterpret_cast<const int&>(fff);
}
*/
