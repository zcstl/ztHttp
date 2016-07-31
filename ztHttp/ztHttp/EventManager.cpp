//


#include <cstring>  //string.h, strlen, memset,,,
#include <unistd.h> //close, chown, ,,
#include <typeinfo>

#include "EventManager.h"


namespace ztHttp {

Reactor::Reactor(EventMultiplexerAbstractClass* em):_em(em) {}

//Reactor
int Reactor::handle_events(){
    return _em->handle_events();
}

int Reactor::register_handler(EventHandlerAbstractClass* handler) {
    return _em->register_handler(handler);
}

int Reactor::remove_handler(EventHandlerAbstractClass* handler) {
    return _em->remove_handler(handler);
}
int Reactor::select() {
    return _em->select();
}

//EpollMultiplexer
EpollMultiplexer::EpollMultiplexer():_is_running(false) {
    pthread_mutex_init(&_mtx, 0);
    pthread_mutex_init(&_mtx_rdy, 0);
    _fd_epoll=epoll_create(EPOLL_CONS);
}

EpollMultiplexer::~EpollMultiplexer() {
    stopAndEnd();//停止并结束复用器
    close(_fd_epoll);
    /**pthread_mutex_destory(&_mtx);
    pthread_mutex_destory(&_mtx_rdy);
    **/
}


bool EpollMultiplexer::stopAndEnd() {
    //
    return true;
}

int EpollMultiplexer::handle_events() {
    //执行期间对rdy一直上锁，若别的T调用select则回被阻塞，影响销量
    //解决方法，rdy1作为缓冲，select放入其中，handle_events先取然处理；
    //pthread_mutex_lock(&_mtx_rdy);
    EpollEventHandler* p_handler;
    for(auto ardy: rdy) {//map的是pair
        cout<<ardy.first<<"  "<<ardy.second<<endl;
        //pthread_mutex_lock(&_mtx);
        p_handler=_fds.find(ardy.first)->second.second;
        //pthread_mutex_unlock(&_mtx);
        cout<<"  tt"<<endl;
        cout<<"  tt"<<endl;

        p_handler->handle_event();
    }
    //pthread_mutex_unlock(&_mtx_rdy);
}

int EpollMultiplexer::register_handler(EventHandlerAbstractClass* p_handler) {
    EpollEventHandler* p_evhandler;
    if(typeid(*p_handler)==typeid(EpollEventHandler)) {
        p_evhandler=dynamic_cast<EpollEventHandler*>(p_handler);
    }else {
        //
        return -1;
    }
    pthread_mutex_lock(&_mtx);
    _fds[p_evhandler->getFd()]=make_pair(p_evhandler->getEvents(), p_evhandler);
    pthread_mutex_unlock(&_mtx);
    epollUpdate(p_evhandler->getFd(), EPOLL_CTL_ADD);
    return 0;
}

int EpollMultiplexer::remove_handler(EventHandlerAbstractClass* p_handler) {
    EpollEventHandler* p_evhandler;
    if(typeid(*p_handler)==typeid(EpollEventHandler)) {
        p_handler=dynamic_cast<EpollEventHandler*>(p_handler);
    }else {
        //
        return -1;
    }
    if(_fds.find(p_evhandler->getFd())==_fds.end()) {
        //
        return 0;
    }
    pthread_mutex_lock(&_mtx);
    _fds.erase(p_evhandler->getFd());
    pthread_mutex_unlock(&_mtx);
    epollUpdate(p_evhandler->getFd(), EPOLL_CTL_DEL);
    return 0;

}

int EpollMultiplexer::select() {

    int msg=0, timeOut=1000;
    struct epoll_event* p_evs=new struct epoll_event[EPOLL_EVNUMS];//异常
    memset(p_evs, 0, sizeof(epoll_event)*EPOLL_EVNUMS);

    //timeOut  ms
    if( (msg=epoll_wait(_fd_epoll, p_evs, EPOLL_EVNUMS, timeOut)) < 0 ) {
        //
        return msg;
    }else if(msg==0) {
        //
        return 0;
    }
    struct epoll_event ev;
    //for(int i=0; i<EPOLL_EVNUMS; ++i) { 内存错误
    for(int i=0; i<msg; ++i) {
        ev=*(p_evs+i);
        pthread_mutex_lock(&_mtx_rdy);
        if(rdy.find(ev.data.fd)!=rdy.end()){
            //warming
        }
        rdy[ev.data.fd]=ev.events;
        pthread_mutex_unlock(&_mtx_rdy);
    }
    return msg;
}

void EpollMultiplexer::epollUpdate(int fd, uint32_t options) {
    struct epoll_event ev;
    ev.data.fd=fd;
    pthread_mutex_lock(&_mtx);
    ev.events=_fds[fd].first;
    pthread_mutex_unlock(&_mtx);
    if(!epoll_ctl(_fd_epoll, options, fd, &ev))
        return;
    //error
}

void* EMTask::run() {
    //run的终止，能否不通过改变Reactor接口的前提下进行？
    //信号？
    int _count=1024;
    while(_count) {
        _evmp->select();
        _evmp->handle_events();
    }

    return nullptr;
}

}
