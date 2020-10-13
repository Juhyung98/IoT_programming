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
        printf("Input message(Q to quit): ");
        fgets(msg, 1000, stdin);    // 키보드 입력
        if(strcmp(msg, "q\n") == 0 || strcmp(msg, "Q\n") == 0) {
            break;
        }
        len = send(s, msg, strlen(msg), 0);    
        if(len == 0) {
            printf("Send: Disconnected\n");
            break;
        }
        //오류처리
        if(len == -1) {
            perror("send");
            break;
        }
        printf("len: %d\n", len);
        len = recv(s, msg, 1000, 0);
        
        //오류처리
        if(len == -1) {
            perror("recv");
            return 0;
        }
        //접속 끊어짐
        else if(len == 0) {
            printf("Disconnected\n");
            break;
        }
        msg[len] = '\0';   //NULL 문자 넣어주기
        printf("echo: %s\n", msg);
    }

    shutdown(s, SHUT_WR);
    // write shutdown하고 받고, 화면 출력 & 접속 끊어지면 Disconnected출력
    while(1){
        len = recv(s, msg, 1000, 0);
        
        //오류처리
        if(len == -1) {
            perror("recv");
            return 0;
        }
        //접속 끊어짐
        else if(len == 0) {
            printf("Disconnected\n");
            break;
        }
        msg[len] = '\0';   //NULL 문자 넣어주기
        printf("echo: %s\n", msg);
    }
    close(s);

    return 0;
}
