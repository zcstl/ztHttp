
#include"pthread_poolv1.h"

#ifndef HTTP_TASK_THREAD
#define HTTP_TASK_THREAD


class HttpTaskThread{
    public:
        HttpTaskThread(int sock):c_sock(sock){}
        void* run(){

        }
    private:
        int c_sock;
};

#endif
