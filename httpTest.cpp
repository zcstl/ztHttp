/**
	多线程http服务器－测试
**/
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
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
	
	cout<<"Test server is running on port: "<<s_port<<endl;
	while(1){
		if((c_sock=accept(s_sock, nullptr, nullptr)) == -1)
			ERRORDIE("main, accpet;");
		create_pthread();
	}
	close(s_sock);
	return 0;		
}
