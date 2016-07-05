#include <list>
#include <vector>
#include <sys/socket.h>
#include "pthread_poolv1.h"

#ifndef EVENT_
#define EVENT_
using namespace std;


class EventHandler{
	public:
		virtual int handle_event()=0;//类定义不完整，没有vtable
};

class HttpEvent:public EventHandler{
	public:
		HttpEvent(int sock):client_sock(sock){}
		int handle_event();
	private:
		int client_sock;
};

class Reactor{
	public:
		Reactor(){}
		~Reactor(){}
		int handle_events();
		int register_handler(EventHandler*  p_handler);
		int remove_handler(EventHandler*  p_handler);
		virtual int select()=0;

		vector<EventHandler*> waitedQueue;
		vector<EventHandler*> selectedQueue;
    protected:
        void test(){
            for(auto tmp: waitedQueue)
                PRINT_MSG(tmp);
        }
};

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

#endif
