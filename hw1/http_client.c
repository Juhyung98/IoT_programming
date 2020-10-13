#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
    int s;
    struct sockaddr_in server_addr;
    int addr_len;
    char msg[2001]="GET / HTTP/1.1\r\nHost: 220.149.84.127\r\nConnection: close\r\n\r\n";
    char buf[1000];
    int len;

    // socket을 만들고
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s==-1) {
        perror("socket");
        return 0;
    }

    // socket 주소를 넣어
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(80);
    //printf(argv[1]);
    inet_aton(argv[1], &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 0;
    }

    send(s, msg, strlen(msg) + 1, 0);

    while(1){
        len = read(s, buf, 1000);
        if(len == -1){
            perror("read");
            return 0;
        }
        else if(len == 0){
            printf("\n\n\nFinished\n");
            break;
        }
        buf[len] = '\0';
        printf("%s", buf);

    }
    
    close(s);

    return 0;
}
