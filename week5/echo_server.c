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
    char buf[1000];
    int len;
    int i;
    char *ip;
    int port;

    s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);   // UDP 용
            // PF_INET : 인터넷이라는 의미
            // SOCK_STREAM : TCP방식 --> SOCK_DGRAM : UDP방식
    if(s==-1) {
        perror("socket");
    }

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);    // UDP port를 여는것
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), 0, 8);

    // bind하고, 주소를 socket에 넣고
    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");                 
        return 0;
    }

    // UDP는 접속 부분이 없기 때문에 listen과 accept가 빠져야 함
   
    addr_len = sizeof(client_addr);

    // 오류가 나거나 접속이 끊겼을 때 while문 탈출
    while(1){
        // UDP에서 data를 받으려면 recvfrom
        // UDP는 누가 보냈는지 신경 안쓰고 그냥 recvfrom하고 있음  => 여러개의 client가 전송해도 동작을 하게 되어있다.
        len = recvfrom (s, buf, 1000, 0, (struct sockaddr*)&client_addr, &addr_len); // addr_len의 주소

        // 실패하면 프로그램 종료
        if(len == -1) {
            perror("recvfrom");
            return 0;
        }
        // 값이 0 이면 접속이 끊긴 것.
        else if(len == 0) {
            printf("Disconnected\n");
            break;
        }

        // 문자열만 오고 NULL문자가 안왔을 것
        buf[len] = '\0';
        ip = inet_ntoa(client_addr.sin_addr);
        port = ntohs(client_addr.sin_port); // port번호를 little endian으로 바꿔서 넣어.
        // buf[len] = '\0'; ASCII code로 NULL문자와 같은 것
        printf("%s:%d : %s\n", ip, port, buf);

        // send는 write의 UDP 버전
        len = sendto(s, buf, len, 0, (struct sockaddr*)&client_addr, addr_len); // socket s로 buf를 받은 만큼(len만큼) 돌려주면 된다. 
                                                                // flag는 0
                                                                // 받은 곳(client_addr)으로 다시 보내면 된다.
                                                                // 받을 때는 주소 (*addr_len)로 넣었지만, 보낼 때는 값(addr_len)을 넣으면 된다.
        //len 만큼 받았기 때문에 보낼 때도 len만큼 보내면 된다.
        // len = write(client_s, buf, len);

        // 실패하면 프로그램 종료
        if(len == -1) {
            perror("write");
            return 0;
        }
        // 값이 0 이면 접속이 끊긴 것.
        // write 에선 disconnected가 안먹는다 (read에서만 된다.)
        // else if(len == 0) {
        //     printf("Disconnected\n");
        //     break;
        // }
    
        // test하는 방법
        // $ nc 54321 -l (listen option)
        // 이쪽으로 오는 모든 메세지를 화면에 출력
        // 다른 화면에 $ nc 127.0.0.1(server의 IP주소) 54321(54321 port로 접속을 하겠다) --> TCP
                    // $ nc 127.0.0.1 54321 -u --> UDP는 -u옵션을 반드시 줘야 한다.
        // 말을 쓰면 반대쪽 화면에 출력된다.

        // UDP는 listen용 socket을 따로 두지 않고, s를 그냥 data전송용으로 사용한다.
        // close(client_s);
    }

    close(s);

    return 0;
}