/**
	多线程http服务器－测试
**/
#include <vector>
#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <gtest/gtest.h>

#include "pthread_poolv1.h"
#include "ztHttp/ztHttp/EventManager.h"
//
#define ERRORDIE(str) {cout<<"Error in: "<<str<<endl; exit(-1);}
/*返回对应ipv4和端口的socket，并且已listen*/

using namespace std;
using namespace ztHttp;

int startUp(int);

int startUp(in_port_t port){
	int s_sock=-1;
	if((s_sock=socket(AF_INET, SOCK_STREAM, 0))==-1)//0
		ERRORDIE("startUp, s_socket;");

	struct sockaddr_in s_name;
	s_name.sin_family=AF_INET;
	s_name.sin_port=htons(port);
	s_name.sin_addr.s_addr=htonl(INADDR_ANY);//INADDR_ANY,htons,htonl,ip32bit

	if(bind(s_sock, (struct sockaddr*)&s_name, sizeof(s_name))==-1)
		ERRORDIE("startUp bind;");

    //port reuse
	char opt=1, *p_opt=&opt;
	if(!setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, p_opt, sizeof(opt))) {
        //
	}

    //keepalive
    bool isKA=true, *p_isKA=&isKA;
	if(!setsockopt(s_sock, SOL_SOCKET, SO_KEEPALIVE, p_isKA, sizeof(isKA))) {
        //
	}

	if(port==0);//获取动态分配的端口号

	if(listen(s_sock, 100)==-1)
		ERRORDIE("startUp listen");

	return s_sock;

}

const auto &a=42;

int start_server(int argc, char* argv[]){
	int err;
	int s_sock=-1, c_sock=-1;//0应该有用
	in_port_t s_port=8080;//端口号0？
	s_sock=startUp(s_port);

	//
	short _count=2;
	//reactor
	vector<Reactor*> ems;
	for(int i=0; i<_count; ++i)
		ems.push_back(new Reactor(new EpollMultiplexer));
    //
	vector<EMTask*> EMTaskss;
	for(int i=0; i<_count; ++i)
		EMTaskss.push_back(new EMTask(ems[i]));
	ThreadPool tp(2);
	tp.startUp();//开启后才能入队
    for(int i=0; i<_count; ++i)
		tp.enqueue(EMTaskss[i]);
	cout<<"Test server is running on port: "<<s_port<<endl;
	int times=0;
	while(1){
		//if((c_sock=accept(s_sock, nullptr, nullptr)) == -1)
		//	ERRORDIE("main, accpet;");
		//分配任务给每个县城i
		//HttpEvent* htev=new HttpEvent(c_sock);
		c_sock=accept(s_sock, nullptr, nullptr);
		epoll_data_t ed;
		ed.fd=c_sock;
        struct epoll_event ee{EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP, ed};
		MSG_PRINT("begin: ");MSG_PRINT(times)
		EventHandlerAbstractClass* eh=
		    new EpollEventHandler(ee, ems[times++%_count]);
		//sleep(1);
		ems[times++%_count]->register_handler(eh);
	}
	close(s_sock);
	//delete
	return 0;
}

template<typename T>
T fooTest(T t1, T t2) {
    return t1+t2;
}

TEST(FooTest, HandleNoneZeroInput) {
    EXPECT_EQ(2, fooTest(1, 1));
    EXPECT_EQ(3, fooTest(1, 1));
    EXPECT_EQ(2, fooTest(1, 1));
}

int main(int argc, char* argv[]) {
    start_server(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
