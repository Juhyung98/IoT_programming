#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main() 
{
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;
    char buf[101];
    int len;
    int i;

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s==-1) {
        perror("socket");
    }

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), 0, 8);

    // bind하고
    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");                 
        return 0;
    }

    // port열었어요
    if(listen(s, 5) == -1) {
        perror("listen");
        return 0;
    }


    addr_len = sizeof(struct sockaddr_in);

    // 접속이 오면 accept되고 새로운 소켓이 나와요
    client_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);
    if (client_s == -1){
        perror("accept");
        return 0;
    }

    int fd;
    fd = open("echo_server.c", O_RDONLY);

    while(1) 
    {
        len = read(fd, buf, 100);
        if(len == -1){
            perror("read");
            close(fd);    
            close(s);
            return 0;
        }
        
        if(len == 0){
            printf("End\n");
            break;
        }

        buf[len] = 0;
        printf("%s", buf);
        write(client_s, buf, len);  // 에러처리 생략
    }
    close(fd);    
    close(s);

    return 0;
}