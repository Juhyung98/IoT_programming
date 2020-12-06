#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>

void zombie_handler(){
	int status;
	pid_t pid;
	pid = wait(&status);
	printf("wait : %d\n",pid);
}


int main(){
	int s, client_s, addr_len;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buf[1001];
	int len;
	fd_set readfds, readtemp;
	int max_fd = 0;
	int result,i;

	struct sigaction action;
	action.sa_handler = zombie_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGCHLD, &action, 0);


	s = socket(PF_INET, SOCK_STREAM, 0);
	if(s == -1){
		perror("socket");
		return 0;
	}	

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(54321);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(server_addr.sin_zero), 0, 8);

	if(bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("bind");
		return 0;
	}

	if(listen(s, 5) == -1){
		perror("listen");
		return 0;
	}

	

	addr_len = sizeof(struct sockaddr_in);

	FD_ZERO(&readfds);
	FD_SET(s, &readfds);
	max_fd = s;

	while(1){
		readtemp = readfds;

		result = select(max_fd+1, &readtemp, NULL, NULL, NULL);
		if(result == -1){
			perror("select");
			return 0;
		}

		for(i = 0; i<max_fd+1; i++){
			if(FD_ISSET(i, &readtemp)){
				if(i == s){
					// listening
					client_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);
					if(client_s == -1){
						switch(errno){
							case EINTR:
								printf("interrupt\n");
								continue;
							default:
								perror("accept");
						}
						return 0;
			
					}
					printf("connected\n");
					FD_SET(client_s, &readfds);
					if(max_fd < client_s){
						max_fd = client_s;
					}
				} else{
					// DATA
					len = recv(i, buf, 1000, 0);
					if(len == -1){
						perror("recv");
						return 0;
					}else if(len == 0){
						FD_CLR(i, &readfds);
						close(i);
						printf("Disconnected\n");
						break;
					}
	
					buf[len] = 0;
					printf("recv : %s\n",buf);

					len = send(i, buf, len, 0);
					if(len == -1){
						perror("send");
						return 0;
					}	
				}
			}		
		}
	}
	close(s);

	return 0;
}
