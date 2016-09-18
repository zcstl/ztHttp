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

//������accept������TcpSocket
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
    /*virtual bool setRecvCallBack(void*(*)(IOBufferAbstractClass*)); ֮ǰ�ģ���磬�����Ǵ��飬��û�ж��壬��û��vtable*/
    virtual bool setRecvCallBack(void*(*)(IOBufferAbstractClass*))=0;

    virtual bool connect(std::string host, uint16_t port)=0;
    virtual bool isConnected()=0;
    virtual int disConnected()=0;
    virtual bool setDisConnectedCallBack(void*(*)(void*))=0;

};


//read,write�׽��֣���ִ��Ԥ��Ļص�����
//��care�Ƿ��������ǿͻ��˵�socket��������Ԫ���socket��һ��
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

    int get_line(char*, int); //��_in_bufferȡһ�з���buf; //size û���ǣ�bug
    void unimplemented(IOBufferAbstractClass*);
    void not_found(IOBufferAbstractClass*);
    void serve_file(const char *filename, IOBufferAbstractClass*);
    void headers(const char *filename, IOBufferAbstractClass* buf);
    void cat(FILE *resource, IOBufferAbstractClass* buf);

    void execute_cgi(int client, const char *path,
        const char *method, const char *query_string);

    vector<char> _in_buffer;


};


//server��client��socket�����һ���Ժ��ٽ������������Ҫ�����ӿڣ�����Ҳ���Ǹ���ѡ��
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
    //�������ӷ����˿ں�Ϊ0��ʾ��̬����
    in_port_t  _port;
    pthread_mutex_t _mtx;
    /*
     * Linux��socket.h
     struct sockaddr {
         sa_family_t   sa_family;       //address family
         char          sa_data[];       //socket address (variable-length data)
                                        //linux��Ϊ14
     }

    �����ԣ�char a[];����������aΪchar[0]û���ڴ棬��Linux�£�sockaddr��sockaddr_in��sizeof��ͬ��
    ��ʵ�ֵĽǶȣ�sockaddrҲ����sizeofΪsa_family_t�Ĵ�С����ȫ����ͨ��len�ж�������socket������static_castת��ָ��ĸо����

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


    //����tcpsocket�����ͣ�����Ϊ�ͻ���ȥconnect��������Ϊ������ɼ����׽��ַ��ص�
    const bool _isServer;
    bool _isConnected;
    char _status;

    TcpSocket(const TcpSocket&);

    friend class TcpListeningSocket;

    Http* _p_http;


};

}

#endif
