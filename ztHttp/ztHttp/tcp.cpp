//
#include <cstring> //memset

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include <glog/logging.h>

#include "tcp.h"

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"
#define STDIN   0
#define STDOUT  1
#define STDERR  2


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

    int fd_sock = -1;
    pthread_mutex_lock(&_mtx);

    if((fd_sock = ::accept(_fd_sock, nullptr, nullptr)) == -1) {//nullptr是Ｃ++11，但放这里也可以

        LOG(INFO)<<"TcpListeningSocket::accept(): error "<<strerror(errno);
        pthread_mutex_unlock(&_mtx);
        return nullptr;

    }

    pthread_mutex_unlock(&_mtx);

    return new TcpSocket(fd_sock);

}


/*
TcpSocket::TcpSocket(in_port_t port): _isServer(false), _port(port),
    _maxRecvSize(1024), _maxSendSize(1024), _isConnected(false) {

    pthread_mutex_init(&_mtx, 0);

    if((_fd_sock = socket(AF_INET, SOCK_STREAM, 0))==-1) {
       //

    }

    memset(&s_addr, 0, sizeof(s_addr));//to wrapper func
    memset(&c_addr, 0, sizeof(c_addr));

    if( port != 0 ) {

        s_addr.sin_family=AF_INET;
        s_addr.sin_port=htons(_port);
        s_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        //bind这种POSI函数或Ｃ标准库函数，其声明和定义都在全局作用域，需::，当然也可以名字查找找到

        if( bind(_fd_sock, reinterpret_cast<struct sockaddr*>(&s_addr), sizeof(s_addr) ) == -1 ) {
            //

        }
    }

    //

}
*/

TcpSocket::TcpSocket(in_port_t fd_connected_sock): _fd_sock(fd_connected_sock),
    _isServer(true), _maxRecvSize(1024), _maxSendSize(1024), _isConnected(true), _status(TCP_NOMAL) {

    pthread_mutex_init(&_mtx, 0);
    socklen_t sl = sizeof(s_addr);

    if(getsockname(_fd_sock, reinterpret_cast<struct sockaddr*>(&s_addr), &sl)) {
        //
    }

    _port = s_addr.sin_port;

    _p_http = new Http();

}


TcpSocket::~TcpSocket() {

   //pthread_mutex_destory(_mtx);

}

int TcpSocket::getFd() {

    return _fd_sock;

}

//对send的封装，阻塞
//将IOBuffer内容写到socket的写缓冲区
ssize_t TcpSocket::write(IOBufferAbstractClass* chunk) {

    if(!_isConnected)
        return -1;

    if(chunk)
        _sendBuffer.append(chunk);

    if(_sendCall)
        _sendBuffer.append(_sendCall(&_sendBuffer));//隐式转换
    _sendCall = nullptr;

    _p_http->write(&_sendBuffer);

    int sz = _maxSendSize<_sendBuffer.size()?_maxSendSize:_sendBuffer.size();

    char* wb = _sendBuffer.pullDown(sz);
    std::cout<<wb<<std::endl;

    int siz = 0;
    if((siz = ::send(_fd_sock, wb, sz, 0)) == -1) {

        LOG(INFO)<<"TcpSocket::write() error: "<<strerror(errno);

    }else if(siz == 0) {

        LOG(INFO)<<"TcpSocket::write():  the size of send buffer is 0";

    }else {

        if(_sendBuffer.consume(siz)) {

            if(!_sendBuffer.size())
                shutdown(_fd_sock, SHUT_WR), _status &= TCP_WR_CLOSED;
                    return siz;
        } else {

            LOG(ERROR)<<"TcpSocket::write(): consume";

        }

    }

    /*
        短链接，若sendbuffer为空，则关闭读端
    */
    if(!_sendBuffer.size())
        shutdown(_fd_sock, SHUT_WR), _status &= TCP_WR_CLOSED;

    return 0;
}


bool TcpSocket::setSendCallBack(SendCallFunc *sendCall) {

    return _sendCall=sendCall;

}

//对read的封装，阻塞；
//将socket的读缓冲区内容copy到IOBuffer
ssize_t TcpSocket::read(IOBufferAbstractClass* p_read_buffer) {

    if(!_isConnected)
        return -1;

    char* rc = new char[_maxRecvSize]{0};

    int siz = ::recv(_fd_sock, rc, _maxRecvSize, 0);
    if(siz >0 ) {
        //_recvBuffer.append(const_cast<IOBuffer*>(&IOBuffer(rc, siz)));temprary的地址不能获取，不管是用底层const接收还是const_cast，
        _recvBuffer.append(new IOBuffer(rc, siz));

    } else if(siz == 0) {

        LOG(INFO)<<"TcpSocket::read()  EOF";

    } else {

        LOG(INFO)<<"TcpSocket::read() "<<strerror(errno);

    }

    if(_recvCall)
        _recvCall(&_recvBuffer);
    //
    _recvCall = nullptr;

    if(!p_read_buffer)
        p_read_buffer = &_recvBuffer;

    if(!_recvBuffer.size())
        shutdown(_fd_sock, SHUT_WR), _status &= TCP_RD_CLOSED;

    _p_http->read(&_recvBuffer);

    return siz;
}

bool TcpSocket::setRecvCallBack(RecvCallFunc *recvCall) {

    return _recvCall = recvCall;

}

bool TcpSocket::connect(std::string host, in_port_t port) {
    /*
    if(_isServer) {
        LOG(ERROR)<<"TcpSocket::connect(): isServer can not connect as host!";
        return false;
    }

    //TCP,UDP 保留端口0；不使用（若发送过程不准备接受回复消息，则可以作为源端口）
    if( port == 0 ){
        //不考虑被动连接方端口为0
        LOG(ERROR)<<"TcpSocket::connect(): port is 0";
        return false;
    }

    int err = 0;

    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(port);

    //success 1, 无效0，出错-1
    if( ( err = inet_pton(AF_INET, host.c_str(), &c_addr.sin_addr) ) == 1 ) {
        //

    } else if(err == 0) {
        _isConnected = false;
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
    */

    return true;

}


bool TcpSocket::isConnected() {

    return _isConnected;

}

int TcpSocket::disConnected() {

    if(_disConnCall)
        _disConnCall(nullptr);

    ::close(_fd_sock);
    _isConnected = false;

    return 0;

}

bool TcpSocket::setDisConnectedCallBack(DisConnCallFunc *disConnCall) {

    return _disConnCall = disConnCall;

}


int Http::read(IOBufferAbstractClass* buf) {

    if( typeid(*buf) == typeid(IOBuffer) ) {

        IOBuffer* p_buf = dynamic_cast<IOBuffer*> (buf);
        char* sta = p_buf->pullDown(p_buf->size());
        vector<char> tmp(sta, sta + p_buf->size());

        p_buf->consume(p_buf->size());

        _in_buffer.swap(tmp);

    } else {

        cout<<"Http::read: typeid error"<<endl;

    }

    //delete data;
    //data = nullptr;

    return _in_buffer.size();
}

int Http::get_line(char* buf, int size) {

    auto iter = _in_buffer.begin();
    int i = 0;
    for ( ; iter < _in_buffer.end() && i < size; ++iter) {
        if (*iter == '\n')
            break;
        buf[i] = *iter;
    }
    buf[++i] = '\0';
    return --i;
}

void Http::unimplemented(IOBufferAbstractClass* buf) {


    if( typeid(*buf) == typeid(IOBuffer) ) { //501 METHOD Not Implemented

        IOBuffer* p_buf = dynamic_cast<IOBuffer*> (buf);
        char msg[] = {"HTTP/1.0 200 OK\r\nServer: jdbhttpd/0.1.0\r\nContent-Type:\
         text/html\r\n\r\n<HTML><HEAD><TITLE>Method Not Implemented\r\n</TITLE></HEAD>\r\n<BODY>\
         <P>HTTP request method not supported.\r\n</BODY></HTML>\r\n"};
        p_buf->append(msg, strlen(msg));

    } else {

        cout<<"sendCallFUnc: typeid error"<<endl;

    }

    //delete data;
    //data = nullptr;
}


void Http::not_found(IOBufferAbstractClass* buf) {

    if( typeid(*buf) == typeid(IOBuffer) ) {

        IOBuffer* p_buf = dynamic_cast<IOBuffer*> (buf); //404 NOT FOUND
        char msg[] = {"HTTP/1.0 200 OK\r\nServer: jdbhttpd/0.1.0\r\nContent-Type: text/html\r\n\
        \r\n<HTML><TITLE>Not Found</TITLE>\r\n<BODY><P>The server could not fulfill\r\n\
        your request because the resource specified\r\nis unavailable or nonexistent.\r\n</BODY></HTML>\r\n"};
        p_buf->append(msg, strlen(msg));

    } else {

        cout<<"sendCallFUnc: typeid error"<<endl;
    }
}


void Http::serve_file(const char *filename, IOBufferAbstractClass* abuf) {

    FILE *resource = NULL;
    int numchars = 1;
    char buf[1024];

    buf[0] = 'A'; buf[1] = '\0';
    while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
        numchars = get_line(buf, sizeof(buf));

    resource = fopen(filename, "r");
    if (resource == NULL)
        not_found(abuf);
    else
    {
        headers(filename, abuf);
        cat(resource, abuf);
    }
    fclose(resource);
}

void Http::headers(const char *filename, IOBufferAbstractClass* buf) {

    if( typeid(*buf) == typeid(IOBuffer) ) {

        IOBuffer* p_buf = dynamic_cast<IOBuffer*> (buf);
        char msg[] = {"HTTP/1.0 200 OK\r\nServer: jdbhttpd/0.1.0\r\nContent-Type: text/html\r\n\r\n"};
        p_buf->append(msg, strlen(msg));

    } else {

        cout<<"sendCallFUnc: typeid error"<<endl;
    }
}

void Http::cat(FILE*resource, IOBufferAbstractClass* buf) {

    if( typeid(*buf) == typeid(IOBuffer) ) {

        IOBuffer* p_buf = dynamic_cast<IOBuffer*> (buf);

        char abuf[1024];

        fgets(abuf, sizeof(abuf), resource);

        while (!feof(resource)) {
            p_buf->append(abuf, strlen(abuf));
            fgets(abuf, sizeof(buf), resource);
        }

    } else {

        cout<<"sendCallFUnc: typeid error"<<endl;
    }

}

int Http::write(IOBufferAbstractClass* abuf) {

    //int client = *(int*)arg;
    char buf[1024];
    size_t numchars;
    char method[255];
    char url[255];
    char path[512];
    size_t i, j;
    struct stat st;
    int cgi = 0;      /* becomes true if server decides this is a CGI
                       * program */
    char *query_string = NULL;

    numchars = get_line(buf, sizeof(buf));
    i = 0; j = 0;
    while (!ISspace(buf[i]) && (i < sizeof(method) - 1))
    {
        method[i] = buf[i];
        i++;
    }

    //
    j=i;
    method[i] = '\0';

    if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
    {
        unimplemented(abuf);
        return -1;
    }

    if (strcasecmp(method, "POST") == 0)
        cgi = 1;

    i = 0;
    while (ISspace(buf[j]) && (j < numchars))
        j++;
    while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < numchars))
    {
        url[i] = buf[j];
        i++; j++;
    }
    url[i] = '\0';

    if (strcasecmp(method, "GET") == 0)
    {
        query_string = url;
        while ((*query_string != '?') && (*query_string != '\0'))
            query_string++;
        if (*query_string == '?')
        {
            cgi = 1;
            *query_string = '\0';
            query_string++;
        }
    }
    //
    sprintf(path, "htdocs%s", url);
    if (path[strlen(path) - 1] == '/')
        strcat(path, "index.html");
    if (stat(path, &st) == -1) {
        while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
            numchars = get_line(buf, sizeof(buf));
        not_found(abuf);
    }
    else
    {
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            strcat(path, "/index.html");
        if ((st.st_mode & S_IXUSR) ||
                (st.st_mode & S_IXGRP) ||
                (st.st_mode & S_IXOTH)    )
            cgi = 1;
        if (!cgi)
            serve_file(path, abuf);
        else
            //execute_cgi(client, path, method, query_string);
            serve_file(path, abuf);
    }

    return 1;
}

}



