#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main() 
{
    int s;
    struct sockaddr_in server_addr;
    int addr_len;
    char msg[1001];
    int len;
    int read_len;

    // socket을 만들고
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s==-1) {
        perror("socket");
        return 0;
    }

    // socket 주소를 넣어
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 0;
    }

    while(1) {
        printf("> ");
        fgets(msg, 1000, stdin);    // 키보드 입력
        len = write(s, msg, strlen(msg));     // 문자열 길이만큼 
        
        //오류처리
        if(len == -1) {
            perror("write");
            close(s);
            return 0;
        }
        while(len > 0) {
            read_len = read(s, msg, 1000);
            //오류처리
            if(len == -1) {
                perror("read");
                close(s);
                return 0;
            }
            //접속 끊어짐
            if(len == 0) {
                printf("Disconnected\n");
                break;
            }
            len -= read_len;
            msg[read_len] = 0;   //NULL 문자 넣어주기
            printf("echo: %s\n", msg);
        }
    }

    close(s);

    return 0;
}
