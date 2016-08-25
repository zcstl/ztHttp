//
#include <cstring> //memset

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp.h"

namespace ztHttp {


TcpListeningSocket::TcpListeningSocket(in_port_t port): _port(port) {

    int err=0;
    pthread_mutex_init(&_mtx, 0);
    //socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)
    if((_fd_sock=socket(AF_INET, SOCK_STREAM, 0))==-1) {
        //socket失败，将停止构造该对象，怎么做？
    }

    _server_name.sin_family=AF_INET;
    _server_name.sin_port=htons(_port);
    _server_name.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(_fd_sock, (struct sockaddr*)&_server_name, sizeof(_server_name))==-1) {
        //
    }

    if(_port==0) {
        //获取动态分配的端口号
    }

    if(listen(_fd_sock, MAXCONNNUM)==-1) {
        //
    }

    //print
}


TcpListeningSocket::~TcpListeningSocket() {
    //print
    close(_fd_sock);
    /**pthread_mutex_destory(_mtx);
    **/
}


TcpSocketAbstractClass* TcpListeningSocket::accept() {
    int fd_sock=-1;
    pthread_mutex_lock(&_mtx);
    if((fd_sock=::accept(_fd_sock, nullptr, nullptr))==-1) {//nullptr是Ｃ++11，但放这里也可以
        //
        pthread_mutex_unlock(&_mtx);
        return nullptr;
    }

    pthread_mutex_unlock(&_mtx);

    return new TcpSocket(fd_sock);
}



TcpSocket::TcpSocket(in_port_t port): _isServer(false), _port(port), _maxRecvSize(1024), _maxSendSize(1024) {

    pthread_mutex_init(&_mtx, 0);

    if((_fd_sock=socket(AF_INET, SOCK_STREAM, 0))==-1) {
       //

    }

    memset(&s_addr, 0, sizeof(s_addr));//to wrapper func
    memset(&c_addr, 0, sizeof(c_addr));

    if(port!=0) {
        s_addr.sin_family=AF_INET;
        s_addr.sin_port=htons(_port);
        s_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        //bind这种POSI函数或Ｃ标准库函数，其声明和定义都在全局作用域，需::，当然也可以名字查找找到
        if(bind(_fd_sock, reinterpret_cast<struct sockaddr*>(&s_addr), sizeof(s_addr))==-1) {
            //

        }
    }

    //

}


TcpSocket::TcpSocket(int fd_connected_sock): _fd_sock(fd_connected_sock),
    _isServer(true), _maxRecvSize(1024), _maxSendSize(1024) {

    pthread_mutex_init(&_mtx, 0);
    socklen_t sl;

    if(getsockname(_fd_sock, reinterpret_cast<struct sockaddr*>(&s_addr), &sl)) {
        //
    }

    _port=s_addr.sin_port;

}


TcpSocket::~TcpSocket() {
   /**pthread_mutex_destory(_mtx);
   **/
}

int TcpSocket::getFd() {
    return _fd_sock;
}

//对send的封装，阻塞
//将IOBuffer内容写到socket的写缓冲区
ssize_t TcpSocket::write(IOBufferAbstractClass* chunk) {

    if(!chunk)
        _sendBuffer.append(chunk);

    if(_sendCall)
        _sendBuffer.append(_sendCall(&_sendBuffer));//隐式转换

    int sz=_maxSendSize<_sendBuffer.size()?_maxSendSize:_sendBuffer.size();
    char* wb=_sendBuffer.pullDown(sz);


    int siz=::send(_fd_sock, wb, sz, 0);
    if(siz==-1) {
        //

    }else if(siz==0) {
        //

    }else {
        if(_sendBuffer.consume(siz)) {
            //
            return siz;
        } else {
            //

        }

    }
    //
    return 0;
}


bool TcpSocket::setSendCallBack(SendCallFunc *sendCall) {
    return _sendCall=sendCall;
}

//对read的封装，阻塞；
//将socket的读缓冲区内容copy到IOBuffer
ssize_t TcpSocket::read(IOBufferAbstractClass* p_read_buffer) {

    char* rc=new char[_maxRecvSize]{0};
    int siz=::recv(_fd_sock, rc, _maxRecvSize, 0);

    if(siz>0) {
        //_recvBuffer.append(const_cast<IOBuffer*>(&IOBuffer(rc, siz)));temprary的地址不能获取，不管是用底层const接收还是const_cast，
        _recvBuffer.append(new IOBuffer(rc, siz));
    }

    if(_recvCall)
        _recvCall(&_recvBuffer);
    //
    if(!p_read_buffer)
        p_read_buffer=&_recvBuffer;

    return false;
}

bool TcpSocket::setRecvCallBack(RecvCallFunc *recvCall) {
    return _recvCall=recvCall;
}

bool TcpSocket::connect(std::string host, in_port_t port) {

    if(_isServer) {
        //
        return false;
    }

    //TCP,UDP 保留端口0；不使用（若发送过程不准备接受回复消息，则可以作为源端口）
    if(port==0){
        //不考虑被动连接方端口为0
        return false;
    }

    int err=0;

    c_addr.sin_family=AF_INET;
    c_addr.sin_port=htons(port);

    //success 1, 无效0，出错-1
    if((err=inet_pton(AF_INET, host.c_str(), &c_addr.sin_addr)) == 1) {
        //

    } else if(err==0) {
        //
        return false;
    } else {
        //
        return false;
    }

    //返回：0 -1；  若fd没有绑定地址，connect会绑定一个默认地址
    if(::connect(_fd_sock, reinterpret_cast<sockaddr*>(&c_addr), sizeof(c_addr))) {
        //
        return false;
    }

    //获取主动连接动态分配的端口
    if(_port==0) {

        //0 -1
        socklen_t sl=0;
        if(getsockname(_fd_sock, reinterpret_cast<struct sockaddr*>(&s_addr), &sl)) {
            //
            return false;
        }

        _port=s_addr.sin_port;
    }

    _isConnected=true;

    return true;

}


bool TcpSocket::isConnected() {
    return _isConnected;
}

int TcpSocket::disConnected() {
    if(_disConnCall)
        _disConnCall(nullptr);
    ::close(_fd_sock);
    return 0;
}

bool TcpSocket::setDisConnectedCallBack(DisConnCallFunc *disConnCall) {
    return _disConnCall=disConnCall;
}



}



