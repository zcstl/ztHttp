/**
	���߳�http������������
**/
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>

#define ERRORDIE(str) {cout<<"Error in: "<<str<<endl; exit(-1)}
/*���ض�Ӧipv4�Ͷ˿ڵ�socket��������listen*/
int startUp(int);


int startUp(in_port_t port){
	int sock=-1;
	if((sock=socket(AF_INET, SOCK_STREAM, 0))==-1)//0
		ERRORDIE("start up, socket()");
	st
}

int main(int argc, char* argv[]){
	int server_sock=-1, client_sock;//0Ӧ������	
	in_port_t server_port=-1;//�˿ں�0��
	server_sock=startUp(server_port);
	
	cout<<"Test server is running on port: "<<server_port<<endl;
	while(1){
		client_sock=accept(server_sock, nullptr, nullptr);
		if(client_sock == -1)
			ERRORDIE("accept error!");
	}
	close(server_sock);
	return 0;		
}
