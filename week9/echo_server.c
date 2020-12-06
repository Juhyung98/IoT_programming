#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

void zombie_handler()
{
    int status;
    pid_t pid;
    pid = wait(&status);
    printf("wait: %d\n", pid);
}

int main() 
{
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;
    char buf[11];
    int len;
 
    struct sigaction action;
    action.sa_handler = zombie_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, 0);

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

    while(1){

        // 접속이 오면 accept되고 새로운 소켓이 나와요
        // accept를 한 다음에 fork를 해야해
        // 자식은 아래의 while(1)을 수행하고, 부모는 fork를 한 뒤에 다시 accept해야해
        client_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);
        if (client_s == -1){
            switch (errno) {
                case EINTR: // 이 상황에서는 다시 accpet를 하도록
                    printf("Interrupt\n");
                    continue;
                
                default:
                    perror("accpet");
            }
            return 0;
        }

        // accept를 하고 fork를 해
        // 자식 프로세스는 0 return
        // 부모 프로세스는 자식 프로세스의 PID를 return


        if(fork() == 0){    // 자식 프로세스 코드
            // 오류가 나거나 접속이 끊겼을 때 while문 탈출
            // 프로세스를 나눈 뒤에 fd 가 2개씩 생기기 때문에 listen하던 socket을 닫아줘야 해
            close(s);

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
        
            close(client_s);
            exit(1);
        }
        else {  // 부모 프로세스 코드
            // fork를 하면 부모프로세스는 딱히 할 일 없이 다시 위로 올라가
            // fork를 하자마자 data통신용 socket을 닫아야해 (이건 자식 프로세스만 사용하니까)
            close(client_s);
        }
    }
    close(s);

    return 0;
}