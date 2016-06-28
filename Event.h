#include <list>
#include <vector>
#include <sys/socket.h>
#include "pthread_poolv1.h"

#ifndef EVENT_
#define EVENT_
using namespace std;


class EventHandler{
	public:
		virtual int handle_event()=0;
};

class HttpEvent{
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
		
		vector<EventHandler> waitedQueue;
		vector<EventHandler> selectedQueue;
};

class HttpEventMultiplex: public Reactor{
	public:
		int select();	
};

class HttpTasks: public ThreadAbstractClass{
	public:
		httpTasks(HttpEventMultiplex* mup):evmp(mup){}
		~httpTasks(){}
		void* run();
	private:
		HttpEventMultiplex* evmp;
};

#endif
