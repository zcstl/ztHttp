#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <list>
#include <vector>
#include <sys/socket.h>

#include "pthread_poolv1.h"



namespace ztHttp{

/**
using namespace std;把std下的名字放到当前作用域
1.在ztHttp作用域引入std，不会影响ztHttp外的作用域
2.此时，ztHttp已包含std的声明
3.故该语句放在namespace里较好
**/
using namespace std;
using std::tr1::function;
using std::tr1::shared_ptr;

/*sizeof该抽象类，大小为8，含虚函数表指针*/
class EventHandler {
	public:
		virtual int handle_event()=0;
		virtual void* get_handle()=0;

};

/**
Reactor
事件管理逻辑与实现解耦，实现委托给EventMultiplexer执行
**/
class Reactor {

	public:
		Reactor(EventMultiplexer em);
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
        EventMultiplexer* _em;
};

/**
EventMultiplexer
负责具体实现的借口，遵循DIP
**/
class EventMultiplexer {
    public:
        int handle_events()=0;
		int register_handler(HandlerType* handler)=0;
		int remove_handler(HandlerType* handler)=0;
		int select()=0;
};

class EpollMultiplexer: public EventMultiplexer {


    public:
        EpollMultiplexer();
        virtual ~EpollMultiplexer();
        int handle_events();
        /**dynamic_cast到目标类型或typeid，若不成功则不能继续注册和移除**/
		int register_handler(HandlerType* handler);
		int remove_handler(HandlerType* handler);
		int select();


    private:
        void epollUpdate();//

		/**
		1.至少该子T和主线程会使用该EventMultiplexer对象
		2.且在多T环境下，结合复用的考虑，不应该假设仅在一个T被使用
		故需锁
		3.其后可以在完整的版本对锁优化（若可以就去掉）；
		**/
		pthread_mutex_t _mtx;

        int _fd_epoll;
        int _fd_event;


        /**类型别名的查找符合名字查找**/
        using Fd=int;


        /**枚举类型的定义方式**/
        /**
        1.事件类型会不会太少？
        2._fds的定义，若只有三种事件类型，且各种类型只有一种动作，则使用list更好
        **/
        enum EventType {
            EM_RE,
            EM_WR,
            EM_ERR
        };


        map<Fd, map<EventType, void(*)()>> _fds;
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
