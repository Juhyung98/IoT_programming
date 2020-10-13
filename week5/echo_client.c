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
    struct sockaddr_in addr;
    int addr_len;
    char msg[1001];
    int len;
    int read_len;

    // socket을 만들고
    s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);   // UDP: SOCK_DGRAM (data gram)
    if(s==-1) {
        perror("socket");
        return 0;
    }

    // socket 주소를 넣어
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    // UDP는 접속이 없으니 connect가 사라져야함. => connect 부분 주석 처리 && len = sendto() && read_len = recvfrom()로 해야함.
    // UDP는 connect이 없어서 제대로 된건지 안된건지 판단할 수 없음
    // 주소를 send할 때마다 넣고 싶지 않다 -> connect()사용
    if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 0;
    }

    addr_len = sizeof(server_addr);

    while(1) {
        printf("> ");
        fgets(msg, 1000, stdin);    // 키보드 입력

        // TCP용
        // len = write(s, msg, strlen(msg));     // 문자열 길이만큼  
        
        // UDP용 (connect() 없을 때)
        // len = sendto(s, msg, strlen(msg), 0, (struct sockaddr*)&server_addr, addr_len);

        // UDP connect()를 했을 때
        len = send(s, msg, strlen(msg), 0);

        //오류처리
        if(len == -1) {
            perror("write");
            close(s);
            return 0;
        }

        // UDP용 (connect() 없을 때)
        //read_len = recvfrom(s, msg, 1000, 0, (struct sockaddr*)&addr_len); 
                                        // flag는 0                 recvfrom 할 때는 주소가 들어가야함 (&addr_len)
        
        // UDP connect()를 했을 때
        read_len = recv(s, msg, 1000, 0);
        //오류처리
        if(read_len == -1) { // TCP의 특징 : msg간의 경계가 없음 => 어디서 끊어질지 모르기 때문에 원하는 byte만큼 읽으려고 이렇게 계산하는 것
            perror("read");
            close(s);
            return 0;
        }
        //접속 끊어짐
        if(read_len == 0) {
            printf("Disconnected\n");
            break;
        }
        msg[read_len] = 0;   //NULL 문자 넣어주기
        printf("echo: %s\n", msg);

/*
        // TCP의 특징 : msg간의 경계가 없음 => 어디서 끊어질지 모르기 때문에 원하는 byte만큼 읽으려고 이렇게 계산하는 것
        while(len > 0) {
            // TCP용
            // read_len = read(s, msg, 1000);
            
            // UDP용
            read_len = recvfrom(s, msg, 1000, 0, (struct sockaddr*)&addr_len); 
                                        // flag는 0                 recvfrom 할 때는 주소가 들어가야함 (&addr_len)
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
            len -= read_len;    // 원하는 byte가 될 때까지 계산 => UDP는 경계가 있기 때문에 이렇게 계산하면서 비교할 필요 없음.
            msg[read_len] = 0;   //NULL 문자 넣어주기
            printf("echo: %s\n", msg);
        }

*/
    }

    close(s);

    return 0;
}
