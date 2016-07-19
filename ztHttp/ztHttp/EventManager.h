#ifndef ZTHTTP_EVENT_MANAGER
#define ZTHTTP_EVENT_MANAGER

#include <list>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <map>
#include <pthread.h>
#include <sys/epoll.h>
#include <pair>

#include "pthread_poolv1.h"

#define EPOLL_CONS 128
#define EPOLL_EVNUMS 32

namespace ztHttp {

/**
using namespace std;把std下的名字放到当前作用域
1.在ztHttp作用域引入std，不会影响ztHttp外的作用域
2.此时，ztHttp已包含std的声明
3.故该语句放在namespace里较好
**/

/**
 * 事件复用器，只负责事件复用,但epoll复用与epoll的接口耦和，感觉这是尽目前最大努力的解耦了
 * **/

using namespace std;
using std::tr1::function;
using std::tr1::shared_ptr;

/*sizeof该抽象类，大小为8，含虚函数表指针*/
class EventHandlerAbstractClass {
	public:
		virtual int handle_event()=0;
		virtual void* get_handle()=0;
};

//epoll事件处理，以fd为单位，一个handler处理一个fd对应所有epoll事件
//构造函数：使用epoll_event,且联合体为fd
class EpollEventHandler: public EventHandlerAbstractClass {
    public:
        EpollEventHandler(struct epoll_event ee): _epoll_event(ee) {

        }
        virtual ~EpollEventHandler(){}
		int handle_event() {
            cout<<"handle_event for fd: "<<getFd()<<", events: "<<_epoll_event.events<<endl;
            return 0;
        }
		void* get_handle() {
            return nullptr;
        }
        int getEpollEvent() {
            return _epoll_event;
        }
        int getFd() {
            return _epoll_event.data;//fd
        }

        int getEvents() {
            return _epoll_event.events;
        }

        bool setRdyEvents(uint32_t events) {
            _rdy_events=events;

        }
    private:
        struct epoll_event _epoll_event;//隐藏数据成员的风格
        int _rdy_events;
        //epoll返回就绪fd，使用eventhandler执行
}


/**
Reactor
事件管理逻辑与实现解耦，实现委托给EventMultiplexer执行
**/
class Reactor {

	public:
		Reactor(EventMultiplexerAbstractClass& em);
		/**析构函数声明为virtual的目的？**/
		virtual ~Reactor();
        /**以下方法委托给EventMultiplexer对象执行**/
		int handle_events();
		/**
		注册和移除的对象的三种思路：
		1.使用EventHandler，get_handler获得相应结构体（包含fd，events以及动作）;
		2.使用decltype(em->getHandlerType())，即由em对象决定；
		3.使用void*，即传入地址，至于地址的解释委托给em对象完成；

		第一种是Reactor模式的一部分，把事件的处理动作以及数据封装到EventHandler中，
		遵循OOP，符合SRP，DIP原则，当然，EventHandler的子类和EventMultiplexer的
		子类配套使用(程序员负责)，原因是EventMultiplexer的select方法，例如使用定时器事件管理和
		使用epoll复用或select复用做的事不同，需要的handler类型也不同；

		int register_handler(decltype(em->getHandlerType()) handler);
		int remove_handler(decltype(em->getHandlerType()) handler);
		第二种方法有个很大的缺点，就是decltype(em->getHandlerType())得到的类型得在
		调用点处可以获得；
		**/
        int register_handler(HandlerType* handler)=0;
		int remove_handler(HandlerType* handler)=0;
		int select();


    protected:
        void test(){
            for(auto tmp: waitedQueue)
                PRINT_MSG(tmp);
        }


    private:
        EventMultiplexerAbstractClass* _em;
};

/**
EventMultiplexer
负责具体实现的借口，遵循DIP
**/
class EventMultiplexerAbstractClass {
    public:
        int handle_events()=0;
		int register_handler(HandlerType* handler)=0;
		int remove_handler(HandlerType* handler)=0;
		int select()=0;
};

class EpollMultiplexer: public EventMultiplexerAbstractClass {


    public:
        EpollMultiplexer();
        virtual ~EpollMultiplexer();
        int handle_events();
        /**dynamic_cast到目标类型或typeid，若不成功则不能继续注册和移除**/
		int register_handler(HandlerType* handler);
		int remove_handler(HandlerType* handler);
		int select();


    private:
        void epollUpdate();//移除和注册都会用到，代码复用

		/**
		1.至少该子T和主线程会使用该EventMultiplexer对象
		2.且在多T环境下，结合复用的考虑，不应该假设仅在一个T被使用
		故需锁
		3.其后可以在完整的版本对锁优化（若可以就去掉）；
		**/
		pthread_mutex_t _mtx;
		pthread_mutex_t _mtx_rdy;

        int _fd_epoll;
        //int _fd_event;//eventfd,linux系统调用，暂不使用

        volatile bool _is_running;

        /**类型别名的查找符合名字查找**/
        using Fd=int;
        using events=uint32_t;

        /**枚举类型的定义方式**/
        /**
        1.事件类型会不会太少？
        2._fds的定义，若只有三种事件类型，且各种类型只有一种动作，则使用list更好
        enum EventType {
            EM_RE,
            EM_WR,
            EM_ERR
        };

        **/
        //注册,移除都是以文件描述符为单位       
        map<Fd, pair<events, EpollEventHandler*>> _fds;
        map<Fd, events> rdy;
        //使用vector<handler*>数据结构存储监听事件
        //不足：粗粒度，依赖于具体类而非接口,查找效率低下
        //vector<EpollEventHandler*> handlers;
        //vector<EpollEventHandler*> rdy;
};





}
#endif



/*
class HttpEventMultiplex: public Reactor{
	public:
		int select();
};


class HttpTasks: public ThreadAbstractClass{
	public:
		HttpTasks(HttpEventMultiplex* mup):evmp(mup){}
		~HttpTasks(){}
		void* run();//类定义完成后才有可能有vtable
	private:
		HttpEventMultiplex* evmp;//遵循dip，这里应该改为父类比较好
        int testSelect();
};

class HttpEvent:public EventHandler{
	public:
		HttpEvent(int sock):client_sock(sock){}
		int handle_event();
	private:
		int client_sock;
};
*/