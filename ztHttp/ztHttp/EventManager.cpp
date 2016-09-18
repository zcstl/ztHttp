//


#include <cstring>  //string.h, strlen, memset,,,
#include <unistd.h> //close, chown, ,,
#include <typeinfo>
#include <sys/socket.h>

#include <glog/logging.h>

#include "zcsIO.h"
#include "EventManager.h"


void accept_request(void *arg);

extern pthread_t signal_handle_thread;

extern vector<ztHttp::EventHandlerAbstractClass*> *p_wait_queue;
extern pthread_mutex_t wait_queue_mtx;


namespace ztHttp {

Reactor::Reactor(EventMultiplexerAbstractClass* em):_em(em) {}

//Reactor
Reactor::~Reactor() {
    delete _em;
    _em=nullptr;
}

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

    //异常终止
    if(quit_wait_flag)
        quit_and_rst();

    //执行期间对rdy一直上锁，若别的T调用select则回被阻塞，影响销量
    //解决方法，rdy1作为缓冲，select放入其中，handle_events先取然处理；
    //pthread_mutex_lock(&_mtx_rdy);
    EpollEventHandler *p_handler=nullptr;

    //sleep(2);

    for(auto ardy: rdy) {//map的是pair

        LOG(INFO)<<"EpollMultiplexer::handle_events(): fd: "
            <<ardy.first<<" events:  "<<ardy.second;

        pthread_mutex_lock(&_mtx);
        //map::find,,,,map::end()
        if(  _fds.find(ardy.first) == _fds.end() ) {

            pthread_mutex_unlock(&_mtx);
            LOG(ERROR)<<"EpollMultiplexer::handle_events(): fd: "
            <<ardy.first<<" not found in _fds ";

        }

        p_handler = _fds.find(ardy.first)->second.second;
        //cout<<_fds.find(ardy.first)->first<<endl;
        pthread_mutex_unlock(&_mtx);

        //uint32_t 赋给 int　　error:   std::bad_typeid, aborted
        //cout<<typeid(ardy.second).name()<<endl;
        //cout<<typeid(uint32_t).name()<<endl;
        //cout<<11<<endl;
        if( !p_handler->setRdyEvents(ardy.second) ) {

            //cout<<22<<endl;
            LOG(ERROR)<<"EpollMultiplexer::handle_events()"
                "  EpollEventHandler::setRdyEvents";

        }

        //cout<<33<<endl;

        p_handler->handle_event();

    }

    rdy.erase(rdy.begin(), rdy.end());
    //for(auto tmp: rdy)
    //    cout<<" EpollMultiplexer::handle_events: test iserase:"
    //    <<tmp.first<<endl;
    //pthread_mutex_unlock(&_mtx_rdy);

    return 0;

}

int EpollMultiplexer::register_handler(EventHandlerAbstractClass* p_handler) {

    //if(register_flag)
    //    return -1;

    EpollEventHandler* p_evhandler = nullptr;

    if( typeid(*p_handler) == typeid(EpollEventHandler) )
        p_evhandler=dynamic_cast<EpollEventHandler*>(p_handler);
    else {
        LOG(INFO)<<"EpollMultiplexer::register_handler(): dynamic_cast false!";
        return -1;
    }

    pthread_mutex_lock(&_mtx);
    _fds[p_evhandler->getFd()]=make_pair(p_evhandler->getEvents(), p_evhandler);
    pthread_mutex_unlock(&_mtx);

    epollUpdate(p_evhandler->getFd(), EPOLL_CTL_ADD);

    return 0;

}

int EpollMultiplexer::remove_handler(EventHandlerAbstractClass* p_handler) {

    if(register_flag)
        return -1;

    LOG(INFO)<<"EpollMultiplexer::remove_handler() ";
    EpollEventHandler* p_evhandler=nullptr;
    if( typeid(*p_handler) == typeid(EpollEventHandler )) {

        /**bug:  p_handler=dynamic_cast<EpollEventHandler*>(p_handler);
            从而使p_evhandler未定义，后文使用p_evhandler，产生未定义的行为
            这种情况的指针，术语是什么？
        **/
        p_evhandler = dynamic_cast<EpollEventHandler*>(p_handler);

    }else {

        LOG(ERROR)<<"EpollMultiplexer::remove_handler():"
            "typeid error: not EpollEventHandler type";
        return -1;

    }
    cout << p_evhandler->getFd() << endl;
    for(auto tmp: _fds)cout << tmp.first << endl;
    if( _fds.find(p_evhandler->getFd()) == _fds.end() ) {
        //
        LOG(ERROR)<<"EpollEventHandler::remove_handler():"
        " error, fd not found in fds";
        return 0;
    }

    pthread_mutex_lock(&_mtx);
    _fds.erase(p_evhandler->getFd());
    pthread_mutex_unlock(&_mtx);

    epollUpdate(p_evhandler->getFd(), EPOLL_CTL_DEL);

    //
    delete p_handler;
    p_handler=nullptr;

    return 0;

}

int EpollMultiplexer::select() {

    if(quit_wait_flag)
        quit_and_rst();


    int msg=0, timeOut=1000;
    struct epoll_event* p_evs=new struct epoll_event[EPOLL_EVNUMS];//1.异常; 2.可优化为成员
    memset(p_evs, 0, sizeof(epoll_event)*EPOLL_EVNUMS);

    //timeOut  ms
    if( (msg=epoll_wait(_fd_epoll, p_evs, EPOLL_EVNUMS, timeOut)) == -1 ) {
        LOG(ERROR)<<"EpollMultiplexer::select() errno: "
            <<msg;
        return msg;
    }else if(msg == 0) {
        LOG(INFO)<<"EpollMultiplexer::select():  0";
        return 0;
    }

    LOG(INFO)<<"EpollMultiplexer::select():  "<<msg;
    struct epoll_event ev;
    //for(int i=0; i<EPOLL_EVNUMS; ++i) { 内存错误
    for(int i = 0; i < msg; ++i) {

        ev = *(p_evs+i);

        pthread_mutex_lock(&_mtx_rdy);
        if( rdy.find(ev.data.fd) != rdy.end() )
            LOG(INFO)<<"EpollMultiplexer::select(): rdy is not empty!!";
        rdy[ev.data.fd] = ev.events;
        pthread_mutex_unlock(&_mtx_rdy);

    }

    return msg;

}

void EpollMultiplexer::epollUpdate(int fd, uint32_t options) {

    int err=0;

    struct epoll_event ev;
    ev.data.fd=fd;

    pthread_mutex_lock(&_mtx);
    ev.events=_fds[fd].first;
    pthread_mutex_unlock(&_mtx);

    //epoll_ctl: 0, -1, errno
    if( (err = epoll_ctl(_fd_epoll, options, fd, &ev)) != 0 )
        LOG(ERROR)<<"EpollMultiplexer::epollUpdate()："<<errno;

    LOG(INFO)<<"EpollMultiplexer::epollUpdate() success for "<<options;

}

void EpollMultiplexer::quit_and_rst() {

    LOG(ERROR)<<"EpollMultiplexer::quit_and_rst()";

    struct linger rst={1, 0};

    pthread_mutex_lock(&_mtx);

    for(auto eh: _fds) {

        if(!setsockopt(eh.first, SOL_SOCKET, SO_LINGER, &rst, sizeof(rst)))
            LOG(ERROR)<<"EpollMultiplexer::quit_and_rst(), setsockopt error";
        if(!close(eh.first))
            LOG(ERROR)<<"EpollMultiplexer::quit_and_rst(), close error";

    }

    _fds.erase(_fds.begin(), _fds.end());
    rdy.erase(rdy.begin(), rdy.end());

    pthread_mutex_unlock(&_mtx);

    if(!--quit_wait_flag) {

        LOG(ERROR)<<"EpollMultiplexer::quit_and_rst(): pthread_kill: SIGUSR2";
        pthread_kill(signal_handle_thread, SIGUSR2);

    }

    pause();

}

void* EMTask::run() {
    //run的终止，能否不通过改变Reactor接口的前提下进行？
    //信号？
    int _count=1024;
    while(_count) {

        _p_reactor->select();

        if(register_flag)
            registerEvents();

        if(quit_wait_flag)
            quit_reactor();

        _p_reactor->handle_events();

    }

    return nullptr;
}

void EMTask::quit_reactor() {

    //

}

void EMTask::registerEvents() {

    pthread_mutex_lock(&wait_queue_mtx);

    while( p_wait_queue->size() ) {

        LOG(INFO)<<"EMTask::registerEvents(): "<<p_wait_queue->size()
            <<" for reactor "<<_p_reactor;
        EventHandlerAbstractClass *p_eh = *--p_wait_queue->end();
        _p_reactor->register_handler(p_eh);
        p_wait_queue->pop_back();
        --register_flag;

    }

    pthread_mutex_unlock(&wait_queue_mtx);

}

//
int EpollEventHandler::handle_event() {

    LOG(INFO)<<"EpollEventHandler::handle_event(): fd: "
        <<getFd()<<", events: "<<_epoll_event.events;

    if( _rdy_events & EPOLLERR ) {
        handle_err();
        _p_reactor->remove_handler(this);
        return 0;
    }

    if( _rdy_events & EPOLLHUP ) {
        handle_hangup();//
        _p_reactor->remove_handler(this);
        return 0;
    }

    if( _rdy_events & EPOLLIN )
        if( !handle_read() )
            if( !_p_tcp->isConnected() ) {
                _p_reactor->remove_handler(this);
                return 0;
            }

    cout<<EPOLLIN<<" "<<EPOLLOUT<<" "<<EPOLLERR<<" "<<EPOLLHUP<<" "<<_rdy_events<<endl;
    //EPOLLERR:4,    EPOLLOUT:8
    if( _rdy_events & EPOLLOUT )
        if( !handle_write() )
            if( !_p_tcp->isConnected() ) {
                _p_reactor->remove_handler(this);
                return 0;
            }
    //简化处理
    _p_tcp->disConnected();
    _p_reactor->remove_handler(this);

    return 0;
}

bool EpollEventHandler::handle_read() {

    cout<<"read"<<endl;
    if(_p_tcp->read(nullptr) == -1)
        return false;
    return true;
    /*
    if((err=::recv(getFd(), p_buf, siz, 0))==0) {
    //
        LOG(INFO)<<"EpollEventHandler::handle_event：　recv(): error"
            <<" for fd: "<<this->getFd()
            <<" errno: "<<err;
        _p_reactor->remove_handler(this);

        return 0;

    } else {
        //
    }
    */

}

bool EpollEventHandler::handle_write() {

    cout<<"write"<<endl;
    if(_p_tcp->write(nullptr) == -1)
        return false;
    cout<<"write end"<<endl;
    return true;

}

bool EpollEventHandler::handle_err() {

    LOG(INFO)<<"EpollEventHandler::handle_err()";
    return true;

}

bool EpollEventHandler::handle_hangup() {
    cout<<"hug"<<endl;
    return true;
}

void* EpollEventHandler::get_handle() {
    return nullptr;
}

int EpollEventHandler::getEpollEvent() {
    return _epoll_event.events;
}

int EpollEventHandler::getFd() {
    return _epoll_event.data.fd;//data是ｕｎｉｏｎ
}

int EpollEventHandler::getEvents() {
    return _epoll_event.events;
}

bool EpollEventHandler::setRdyEvents(uint32_t events) {

    //lock
    cout<<"----b  events"<<endl;
    _rdy_events = events;
    cout<<"----e  events"<<endl;

    return true;

}


}
