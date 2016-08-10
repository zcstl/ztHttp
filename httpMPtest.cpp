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
#include <glog/logging.h>
//#include <glog/log_severity.h>

#include "ztHttp/ztHttp/zcsIO.h"
#include "pthread_poolv1.h"
#include "ztHttp/ztHttp/EventManager.h"

#define BACKLOG 128

//
//#define ERRORDIE(str) {cout<<"Error in: "<<str<<endl; exit(-1);}
/*返回对应ipv4和端口的socket，并且已listen*/

using namespace std;
using namespace ztHttp;

int start_up(int);

int start_up(in_port_t &port){

	int s_sock=-1;
	if((s_sock=socket(AF_INET, SOCK_STREAM, 0))==-1)//0
		LOG(ERROR)<<"start_up: socket()";

	struct sockaddr_in s_name;
	s_name.sin_family=AF_INET;
	s_name.sin_port=htons(port);
	s_name.sin_addr.s_addr=htonl(INADDR_ANY);//INADDR_ANY,htons,htonl,ip32bit

	if(bind(s_sock, (struct sockaddr*)&s_name, sizeof(s_name))==-1)
		LOG(ERROR)<<"start_up bind()";

    //port reuse
	char opt=1, *p_opt=&opt;
	//retunr 0, -1
	if(setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, p_opt, sizeof(opt)))
        LOG(INFO)<<"start_up: setsockopt(), SO_REUSEADDR not set!";

    //keepalive
    bool isKa=true, *p_isKa=&isKa;
	if(setsockopt(s_sock, SOL_SOCKET, SO_KEEPALIVE, p_isKa, sizeof(isKa)))
        LOG(INFO)<<"start_up: setsockopt(), SO_KEEPALIVE not set!";

	//update port with the port which is dynamically allocated
    if(!port) {
        socklen_t s_name_len=0;
        //return 0, -1;    not static_cast!!
        if(getsockname(s_sock, reinterpret_cast<struct sockaddr* >(&s_name), &s_name_len))
            LOG(ERROR)<<"start_up: getsockname()";
        port=ntohs(s_name.sin_port);
    }

    //return 0, -1
	if(listen(s_sock, BACKLOG))
		LOG(ERROR)<<"start_up: listen()";

	return s_sock;

}

int start_server(int argc, char* argv[]){

	int err;
	int s_sock=-1, c_sock=-1;//0应该有用
	//in_port_t s_port=8080;
    //0: dynamically allocate
	in_port_t s_port=0;
	s_sock=start_up(s_port);

	//the number of threads, reactor; one reactor per thread
	short _count=2;
	//reactor
	vector<Reactor*> reactors;
	for(int i=0; i<_count; ++i)
		reactors.push_back(new Reactor(new EpollMultiplexer));

    //EMTasks run the reactor all the time until captures a signal or program exit;
    //a EMTasks per thread
	vector<EMTask*> EMTaskss;
	for(int i=0; i<_count; ++i)
		EMTaskss.push_back(new EMTask(reactors[i]));

    //
	ThreadPool tp(2);
	tp.startUp();
    for(int i=0; i<_count; ++i)
		tp.enqueue(EMTaskss[i]);

	LOG(INFO)<<"Test server is running on port: "<<s_port;

	int reactor_select=0, times=0;
	while(1){

		reactor_select=++reactor_select%_count;

        //blocking listening socket
        //return c_sock, -1
		if((c_sock=accept(s_sock, nullptr, nullptr))!=-1) {

            epoll_data_t ed;
	    	ed.fd=c_sock;
            struct epoll_event ee{EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP, ed};
		
            VLOG(6)<<"start_server:  accept() "<<times++;
		    EventHandlerAbstractClass* eh=
		        new EpollEventHandler(ee, reactors[reactor_select]);
		    
            /**may be blocked**/
		    reactors[reactor_select]->register_handler(eh);

        } else
            LOG(INFO)<<"start_server: accept(): broken conn";

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
    google::InitGoogleLogging(argv[0]);//使用-libglog无法找到，最后直接添加.so；为什么找不到，，，
    //testing::InitGoogleTest(&argc, argv);
    LOG(INFO)<< "Found" << 1 <<"NUM_SEVERITIES";
    //return RUN_ALL_TESTS();
}

/*
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int getMin(vector<int> &vals, int dest);


int main() {
    int n=0, x=0;
    //string ns, xs;
    cin>>n>>x;
    //cin>>ns>>xs;
    //n=stoi(ns);
    //x=stoi(xs);
    //
    vector<int> vals(n, 0);
    for(int i=1; i<n; ++i)
        cin>>vals[i];
    cout<<getMin(vals, x)<<endl;
}

int getMin(vector<int> &vals, int dest) {
    sort(vals.begin(), vals.end());
    int total=0;
    for(auto tmp: vals) {
        if(total>=dest)
            return total;
        total+=tmp;
    }
    if(total>=dest)
        return total;
    return -1;

}*/
