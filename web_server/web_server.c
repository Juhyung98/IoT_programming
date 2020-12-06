#include <stdio.h> //표준입출력라이브러리
#include <sys/types.h> //시스템관련 라이브러리
#include <sys/socket.h> //네트워크통신 라이브러리
#include <arpa/inet.h> //버클리소켓사용 라이브러리
// #include <netinet/in.h> //인터넷주소체계 사용 라이브러리
#include <string.h> //문자열처리 라이브러리
#include <stdlib.h> //표준입출력라이브러리
#include <unistd.h> //fork사용라이브러리
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>  // perror 출력

#define BUF_SIZE 1024
#define SMALL_BUF 100


void send_data(int client_s, char* content_type, char* file_name);
char* contentType(char* content);
void send_error(int client_s);

int main(int argc, char *argv[]) 
{
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;
    int port;
    char buf[BUF_SIZE];
    char file_name[30];
    char file_name_temp[30];
    char command[10];
    char content_type[15];
    pid_t pid;
    int len;

    if(argc!=2) {    // 사용법
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    
    // socket을 만들고
    s = socket(PF_INET, SOCK_STREAM, 0);
    if(s==-1) {
        perror("socket");
        return 0;
    }

    // socket 주소를 넣어
    server_addr.sin_family = PF_INET;
    port = atoi(argv[1]);
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //printf(argv[1]);
    
    memset(&(server_addr.sin_zero), 0, 8);

    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");                 
        return 0;
    }

    // port열었어요
    if(listen(s, 20) == -1) {
        perror("listen");
        return 0;
    }

  
    while(1){
        addr_len = sizeof(struct sockaddr_in);

        // 접속이 오면 accept되고 새로운 소켓이 나와요
        client_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);
        
        if (client_s == -1){
            perror("accept\n");
            return 0;
        }

        printf("Waiting.....\n\n");
        printf("Connection Request from %s : %d (PID %d)\n", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), pid);

        pid = fork();   

        if(pid == 0) {  // 자식 프로세스 
            close(s);

            len = recv(client_s, buf, BUF_SIZE, 0);

            if(len == -1){
                perror("read\n");
                return 0;
            }


            if(len == 0){
                perror("Disconnected\n");
                break;
            }


			if (strstr(buf, "HTTP/") == NULL) {
				send_error(client_s);
				close(client_s);
				return 1;
			}
      
			strcpy(command, strtok(buf, " "));
			if (strcmp(command, "GET") != 0) {
				//send_error(client_s);
				close(client_s);
				return 1;
			}

            printf("command : %s\n", command);

			strcpy(file_name, strtok(NULL, " "));
            printf("file_name: %s\n", file_name);
            if(strcmp(file_name, "/") == 0) {
                strcpy(file_name, "index.html");
            }
            if(file_name[0] == '/') {
                strcpy(file_name_temp, &(file_name[1]));
            }
            else {
	    		strcpy(file_name_temp, file_name);
            }
            strcpy(file_name, file_name_temp);
			strcpy(content_type, contentType(file_name_temp));
			send_data(client_s, content_type, file_name);
            // close(client_s);
            // exit(0);
        }
        else{
            printf("why\n\n");
        }
        close(client_s);
    }
    close(s);
    return 0;
}

void send_data(int client_s, char* content_type, char* file_name) {
	
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server: Linux Web Server\r\n";
	char connection[] = "Connection: keep-alive\r\n";
	char buf[BUF_SIZE];
    char content_length[SMALL_BUF];
    char content_Type[SMALL_BUF];
    int send_file;
    int fsize;
    int len;

    struct stat sbuf;
	
    send_file = open(file_name, O_RDONLY);
	if(send_file < 0) {
        printf("can not open %s\n", file_name);
		send_error(client_s);
		return;
	}
	
    if(stat(file_name, &sbuf) == -1){
		perror("stat\n");
	}
	else{
		fsize = sbuf.st_size;
	}
	
    printf("====================Reply Message====================\n");
	sprintf(content_length, "Content-Length: %d\r\n", fsize);
	sprintf(content_Type, "Content-Type: %s\r\n\r\n", content_type);
	
    send(client_s, protocol, strlen(protocol), 0);
    write(1, protocol, strlen(protocol));
	send(client_s, server, strlen(server), 0);
	write(1, server, strlen(server));
    send(client_s, content_length, strlen(content_length), 0);
    write(1, content_length, strlen(content_length));
	send(client_s, content_Type, strlen(content_Type), 0);
	write(1, content_Type, strlen(content_Type));

    while ((len = read(send_file, buf, BUF_SIZE)) > 0) {
		send(client_s, buf, len, 0);
	}
	printf("send successful\n");
	// close(client_s);
	
}

char* contentType(char* content){
    char extension[SMALL_BUF];
	char file_name[SMALL_BUF];
	
	strcpy(file_name, content);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));
	
	if(!strcmp(extension, "html")||!strcmp(extension, "htm")) 
		return "text/html";
	else if(!strcmp(extension, "jpg"))
		return "image/jpg";
	else if(!strcmp(extension, "png"))
		return "image/png";
}


void send_error(int client_s) {
	char protocol[] = "HTTP/1.1 404 Not Found\r\n";
	char server[] = "Server:Linux Web Server \r\n";
	
	char content_type[] = "Content-Type: text/html\r\n\r\n";
	char content[] = "<html><head><title>NETWORK</title></head><body><font size=+5><br>error! check request file name or request method!</font></body></html>";
	int len;
    char content_length[SMALL_BUF];
    
    sprintf(content_length, "Content-Length: %d\r\n", (int)strlen(content));
    
	len = send(client_s, protocol, strlen(protocol), 0);
    if(len== -1) {
        perror("send");
    }
	len = send(client_s, server, strlen(server), 0);
    if(len== -1) {
        perror("send");
    }
	len = send(client_s, content_length, strlen(content_length), 0);
    if(len== -1) {
        perror("send");
    }
	len = send(client_s, content_type, strlen(content_type), 0);
    if(len== -1) {
        perror("send");
    }
	len = send(client_s, content, strlen(content), 0);
    if(len== -1) {
        perror("send");
    }
	close(client_s);
}

