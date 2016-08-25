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
#include <pthread.h>
#include <signal.h>

#include <gtest/gtest.h>
#include <glog/logging.h>
//#include <glog/log_severity.h>

#include "ztHttp/ztHttp/zcsIO.h"
#include "pthread_poolv1.h"
#include "ztHttp/ztHttp/EventManager.h"
#include "ztHttp/ztHttp/tcp.h"
#include "IOBuffer.h"

#define BACKLOG 128
#define BACKLOG_Queue 32

//
//#define ERRORDIE(str) {cout<<"Error in: "<<str<<endl; exit(-1);}
/*返回对应ipv4和端口的socket，并且已listen*/

using namespace std;
using namespace ztHttp;

/*
 *使用signao实现事件的异步注册
 * */
//extern sigset_t mask;
//void* signal_handle(void*);

//多个reactors争用一个vector
vector<EventHandlerAbstractClass*> *p_wait_queue = nullptr;
pthread_mutex_t wait_queue_mtx = PTHREAD_MUTEX_INITIALIZER;

//vector<EventHandlerAbstractClass*> *p_wait_queue0=nullptr;
//vector<EventHandlerAbstractClass*> *p_wait_queue1=nullptr;

//pthread_mutex_t wait_queue0_mtx=PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t wait_queue1_mtx=PTHREAD_MUTEX_INITIALIZER;

pthread_t signal_handle_thread;

//using SendCallFunc=IOBufferAbstractClass*(void*);
//using RecvCallFunc=void*(IOBufferAbstractClass*);
//using DisConnCallFunc=void*(void*);

IOBufferAbstractClass* sendCallFunc(void *buf) {

    IOBuffer *p_buf=nullptr;
    char *data=new char[6]{'h','e','l','l','o','\0'};
    IOBufferAbstractClass *_buf = static_cast<IOBufferAbstractClass*> (buf);

    if( typeid(*_buf) == typeid(IOBuffer) ) {

        p_buf = dynamic_cast<IOBuffer*> (_buf);
        p_buf->append(data, 6);
        cout<<p_buf<<endl;

    } else {
        cout<<"sendCallFUnc: typeid error"<<endl;
    }

}



int start_up(int);
void enqueue_and_wait(EventHandlerAbstractClass *eh, int reactor_select);

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


void sig_tran(int signo) {

    LOG(INFO)<<"sig_tran()";
    pthread_kill(signal_handle_thread, signo);

}

int start_server(int argc, char* argv[]){

    int err=0;

    //signal
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    z_signal(SIGINT, sig_tran);
    z_signal(SIGUSR1, sig_tran);
    z_signal(SIGUSR2, sig_tran);

    //if((err=pthread_sigmask(SIG_BLOCK, &mask, 0)) != 0)
    //    LOG(FATAL)<<"start_up: pthread_sigmask(), error";

    //signal handle pthread
    pthread_create(&signal_handle_thread, nullptr, signal_handle, nullptr);

    //不同linux版本，signal的实现不同，故使用sigaction
    //z_signal(SIGUSR1, sig_usr1);

    int s_sock=-1, c_sock=-1;//0应该有用
	//in_port_t s_port=8080;
    //0: dynamically allocate
	in_port_t s_port=8000;
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
    ThreadPool tp(_count);
	tp.startUp();
    for(int i=0; i<_count; ++i)
		tp.enqueue(EMTaskss[i]);

	LOG(INFO)<<"Test server is running on port: "<<s_port;

    /*//wait queue
    p_wait_queue0=new vector<EventHandlerAbstractClass*>;
    p_wait_queue1=new vector<EventHandlerAbstractClass*>;
    p_wait_queue0->reserve(BACKLOG_Queue);
    p_wait_queue1->reserve(BACKLOG_Queue);
    */

    p_wait_queue=new vector<EventHandlerAbstractClass*>;
    p_wait_queue->reserve(BACKLOG_Queue);

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
            //若使用http则需要new TcpSocket设置回然后传给handle类
            TcpSocketAbstractClass *p_tcp = new TcpSocket(c_sock);
            p_tcp->setSendCallBack(sendCallFunc);
		    EventHandlerAbstractClass* eh=
		        new EpollEventHandler(ee, reactors[reactor_select], p_tcp);

            enqueue_and_wait(eh, reactor_select);
		    //reactors[reactor_select]->register_handler(eh);

        } else
            LOG(INFO)<<"start_server: accept(): broken conn";

	}

	close(s_sock);
	//delete
	return 0;
}


void enqueue_and_wait(EventHandlerAbstractClass *eh, int reactor_select) {

    pthread_mutex_lock(&wait_queue_mtx);

    if( p_wait_queue->size() == BACKLOG_Queue ) {

        LOG(INFO)<<"enqueue_and_wait(), full and discard the conn";
        pthread_mutex_unlock(&wait_queue_mtx);
        return;
    }

    p_wait_queue->push_back(eh);
    pthread_mutex_unlock(&wait_queue_mtx);

    pthread_kill(signal_handle_thread, SIGUSR1);

}



int main(int argc, char* argv[]) {

    testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);//使用-libglog无法找到，最后直接添加.so；为什么找不到，，，

    start_server(argc, argv);

    //LOG(INFO)<< "Found" << 1 <<"NUM_SEVERITIES";
    return RUN_ALL_TESTS();
}



/*-------------------------------------------------------------------------------------------*/
/*
class Queue_Register{

    using ee=pair<epoll_event, EventHandlerAbstractClass*>;

 public:

    static Queue_Register* instance() {

        if(_instance==nullptr)
            _instance=new Queue_Register;
        return _instance;

    }

    ee getElement() {
        return reactor1_wait_for_register_pool.pop_back();
    }

 private:

    Queue_Register():  {

        pthread_mutex_init(&reactor1_wait_mtx, 0);
        pthread_mutex_init(&reactor2_wait_mtx, 0);

    }

    vector<pair<epoll_event, EventHandlerAbstractClass*>> reactor1_wait_for_register_pool(BACKLOG/4);
    vector<pair<epoll_event, EventHandlerAbstractClass*>> reactor2_wait_for_register_pool(BACKLOG/4);
    pthread_mutex_t reactor1_wait_mtx;
    pthread_mutex_t reactor2_wait_mtx;

    static Queue_Register *_instance=nullptr;

}
*/
/**
 *not signal version
 * **/
/*
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

		    reactors[reactor_select]->register_handler(eh);

        } else
            LOG(INFO)<<"start_server: accept(): broken conn";

	}

	close(s_sock);
	//delete
	return 0;
}
*/
/*
void enqueue_and_wait1(EventHandlerAbstractClass *eh, int reactor_select) {

    switch (reactor_select) {

        case 0:

            pthread_mutex_lock(&wait_queue0_mtx);

            if(p_wait_queue0->size()==BACKLOG_Queue) {
                VLOG(4)<<"enqueue_and_wait(), full and discard the conn";
                pthread_mutex_unlock(&wait_queue0_mtx);
                return;
            }

            p_wait_queue0->push_back(eh);
            pthread_mutex_unlock(&wait_queue0_mtx);

            pthread_kill(signal_handle_thread, SIGUSR1);
            break;

        case 1:

            pthread_mutex_lock(&wait_queue1_mtx);

            if(p_wait_queue1->size()==BACKLOG_Queue) {
                VLOG(4)<<"enqueue_and_wait(), full and discard the conn";
                pthread_mutex_unlock(&wait_queue1_mtx);
                return;
            }

            p_wait_queue1->push_back(eh);
            pthread_mutex_unlock(&wait_queue1_mtx);

            pthread_kill(signal_handle_thread, SIGUSR1);
            break;

        default:
            LOG(ERROR)<<"enqueue_and_wait() error for: "<<reactor_select;
    }

}
*/

/*-------------------------------------------------------------------------------------------*/

/*
template<typename T>
T fooTest(T t1, T t2) {
    return t1+t2;
}

TEST(FooTest, HandleNoneZeroInput) {
    EXPECT_EQ(2, fooTest(1, 1));
    EXPECT_EQ(3, fooTest(1, 1));
    EXPECT_EQ(2, fooTest(1, 1));
}
*/



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
