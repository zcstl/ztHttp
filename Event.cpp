#include "Event.h"
#include <unistd.h>
#include "ztHttp/ztHttp/printMsg.h"

int Reactor::register_handler(EventHandler*  p_handler){
	waitedQueue.push_back(p_handler);
	return 0;
}

int Reactor::remove_handler(EventHandler*  p_handler){
	//vector<EventHandler>::iterator iter;
	int flag=0;
	for(auto iter=waitedQueue.begin() ; iter<waitedQueue.end(); ++iter)//:  not ::,  for each得到每个元素而不是迭代器
		if(*iter==p_handler)
			flag=1, waitedQueue.erase(iter);
	if(flag)
		return 0;
	MSG_PRINT("Reactor::remove_handler: do not have this handler!");
	return 1;
}

int Reactor::handle_events(){
	int err=0;
	for(auto iter:selectedQueue)
		if(int msg=iter->handle_event()){
			MSG_PRINT("Reactor::handle_events: handle_event error warming!");
            err=1;
    }
	if(err)
		return 1;
	return 0;
}

int HttpEvent::handle_event(){
	MSG_PRINT("This part deal with the http requist and response ");
	sleep(1);
	return 0;
}

void* HttpTasks::run(){
    testSelect();
}

int HttpTasks::testSelect(){
    while(1){
        evmp->select();
    }
}

int HttpEventMultiplex::select(){
    test();
    for(auto tmp: waitedQueue)
        selectedQueue.push_back(tmp);
    handle_events();
}
