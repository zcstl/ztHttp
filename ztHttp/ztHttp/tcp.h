#ifndef ZTHTTP_TCP
#define ZTHTTP_TCP

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <string>

#include "../../IOBuffer.h"

#define MAXCONNNUM  128

#define TCP_NOMAL 0
#define TCP_RD_CLOSED 1
#define TCP_WR_CLOSED 2
#define TCP_CLOSED 3


namespace ztHttp {

class TcpSocketAbstractClass;

//监听，accept，返回TcpSocket
class TcpListeningSocketAbstractClass {
public:
    virtual TcpSocketAbstractClass* accept()=0;
};


class TcpSocketAbstractClass {

public:

    virtual int getFd()=0;

    virtual ssize_t write(IOBufferAbstractClass*)=0;
    virtual bool setSendCallBack(IOBufferAbstractClass*(*)(void*))=0;

    virtual ssize_t read(IOBufferAbstractClass*)=0;
    /*virtual bool setRecvCallBack(void*(*)(IOBufferAbstractClass*)); 之前的ｂｕｇ，若不是纯虚，且没有定义，则没有vtable*/
    virtual bool setRecvCallBack(void*(*)(IOBufferAbstractClass*))=0;

    virtual bool connect(std::string host, uint16_t port)=0;
    virtual bool isConnected()=0;
    virtual int disConnected()=0;
    virtual bool setDisConnectedCallBack(void*(*)(void*))=0;

};


//read,write套接字，可执行预设的回调函数
//不care是服务器还是客户端的socket，对于四元组的socket都一样
class TcpListeningSocket: public TcpListeningSocketAbstractClass {

public:

    TcpListeningSocket(in_port_t  port);
    virtual ~TcpListeningSocket();

    TcpSocketAbstractClass* accept();


private:

    int _fd_sock;
    in_port_t _port;
    struct sockaddr_in _server_name;
    pthread_mutex_t _mtx;


    TcpListeningSocket(const TcpListeningSocket&);
};


using SendCallFunc=IOBufferAbstractClass*(void*);
using RecvCallFunc=void*(IOBufferAbstractClass*);
using DisConnCallFunc=void*(void*);


class Http{

public:
    int read(IOBufferAbstractClass*);
    int write(IOBufferAbstractClass*);

private:

    int get_line(char*, int); //从_in_buffer取一行放入buf; //size 没考虑，bug
    void unimplemented(IOBufferAbstractClass*);
    void not_found(IOBufferAbstractClass*);
    void serve_file(const char *filename, IOBufferAbstractClass*);
    void headers(const char *filename, IOBufferAbstractClass* buf);
    void cat(FILE *resource, IOBufferAbstractClass* buf);

    void execute_cgi(int client, const char *path,
        const char *method, const char *query_string);

    vector<char> _in_buffer;


};


//server和client的socket耦和在一起，以后再解耦，但这样就需要两个接口，所以也不是个好选择
class TcpSocket: public TcpSocketAbstractClass {

public:

    //TcpSocket(uint16_t port);
    TcpSocket(in_port_t fd_connected_sock);

    ~TcpSocket();

    int getFd();

    ssize_t write(IOBufferAbstractClass*);
    bool setSendCallBack(IOBufferAbstractClass*(*)(void*));

    ssize_t read(IOBufferAbstractClass*);
    bool setRecvCallBack(void*(*)(IOBufferAbstractClass*));

    bool connect(string host, uint16_t port);
    bool isConnected();
    int disConnected();
    bool setDisConnectedCallBack(void*(*)(void*));


private:

    int _fd_sock;
    //主动连接方，端口号为0表示动态分配
    in_port_t  _port;
    pthread_mutex_t _mtx;
    /*
     * Linux中socket.h
     struct sockaddr {
         sa_family_t   sa_family;       //address family
         char          sa_data[];       //socket address (variable-length data)
                                        //linux中为14
     }

    经测试：char a[];编译器定义a为char[0]没有内存，且Linux下，sockaddr与sockaddr_in的sizeof相同；
    从实现的角度，sockaddr也可以sizeof为sa_family_t的大小，完全可以通过len判断是哪种socket，和用static_cast转换指针的感觉差不多

    struct sockaddr_in {
        sa_family_t     sin_family;   //AF_INET.
        in_port_t       sin_port;     //Port number.
        struct in_addr  sin_addr;     //IP address.
    }

    struct in_addr {
        in_addr_t  s_addr;
    }

    NADDR_ANYi      //IPv4 local host address.
    INADDR_BROADCAST        //IPv4 broadcast address.
     * */
    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;

    //callback
    SendCallFunc *_sendCall;
    RecvCallFunc *_recvCall;
    DisConnCallFunc *_disConnCall;

    //IO
    IOBuffer _recvBuffer;
    IOBuffer _sendBuffer;
    unsigned _maxRecvSize;
    unsigned _maxSendSize;


    //控制tcpsocket的类型，是作为客户端去connect，还是作为服务端由监听套接字返回的
    const bool _isServer;
    bool _isConnected;
    char _status;

    TcpSocket(const TcpSocket&);

    friend class TcpListeningSocket;

    Http* _p_http;


};

}

#endif
