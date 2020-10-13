#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main() 
{
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;
    char buf[1001];
    int len;
 
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

    // 오류가 나거나 접속이 끊겼을 때 while문 탈출
    while(1){
        len = recv(client_s, buf, 1000, 0);
        // 실패하면 프로그램 종료
        if(len == -1) {
            perror("read");
            return 0;
        }
        // 값이 0 이면 접속이 끊긴 것.
        if(len == 0) {
            printf("Disconnected\n");
            break;
        }

        // 문자열만 오고 NULL문자가 안왔을 것
        buf[len] = '\0';
        // buf[len] = '\0'; ASCII code로 NULL문자와 같은 것
        printf("%s", buf);
        send(client_s, buf, len, 0);
    }
    
    // test하는 방법
    // $ nc 54321 -l (listen option)
    // 이쪽으로 오는 모든 메세지를 화면에 출력
    // 다른 화면에 $ nc 127.0.0.1(server의 IP주소) 54321(54321 port로 접속을 하겠다)
    // 말을 쓰면 반대쪽 화면에 출력된다.
  
    write(client_s, "Goodbye", 7);  // 접속 끊은 다음에 보내

    close(client_s);
    close(s);

    return 0;
}