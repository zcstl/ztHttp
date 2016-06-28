/**
	多线程http服务器－测试
**/
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#include "pthread_poolv1.h"
#include "Event.h"
#include <vector>
//
#include <iostream>
#define ERRORDIE(str) {cout<<"Error in: "<<str<<endl; exit(-1);}
/*返回对应ipv4和端口的socket，并且已listen*/

using namespace std;

int startUp(int);


int startUp(in_port_t port){
	int s_sock=-1;
	if((s_sock=socket(AF_INET, SOCK_STREAM, 0))==-1)//0
		ERRORDIE("startUp, s_socket;");
	struct sockaddr_in s_name;
	s_name.sin_family=AF_INET;
	s_name.sin_port=htons(port);
	s_name.sin_addr.s_addr=htonl(INADDR_ANY);//INADDR_ANY,htons,htonl,ip32bit
	if(bind(s_sock, (struct sockaddr*)&s_name, sizeof(s_name))==-1)
		ERRORDIE("startUp bind;");
	if(port==0);//获取动态分配的端口号
	if(listen(s_sock, 100)==-1)
		ERRORDIE("startUp listen");
	return s_sock;

}

int main(int argc, char* argv[]){
	int err;
	int s_sock=-1, c_sock=-1;//0应该有用
	in_port_t s_port=-1;//端口号0？
	s_sock=startUp(s_port);

	//	
	short count=2;
	vector<HttpEventMultiplex> httpEvs;
	for(int i=0; i<count; ++i)
		httpEvs.push_back(new HttpEventMultiplex);
	vector<HttpTasks> httpTasks;
	for(int i=0; i<count; ++i)
		httpTasks.push_back(new HttpTasks(httpEvs[i]));
	ThreadPool tp(2);
	for(int i=0; i<count; ++i)
		tp.enqueue(httpTasks[i]);
	tp.startUp();
	cout<<"Test server is running on port: "<<s_port<<endl;
	int times=0;
	while(1){
		if((c_sock=accept(s_sock, nullptr, nullptr)) == -1)
			ERRORDIE("main, accpet;");
		//分配任务给每个县城i
		HttpEvent* htev=new HttpEvent(c_sock);
		httpEvs[times++%count]->register_handler(htev); 
	}
	close(s_sock);
	//delete 
	return 0;
}
