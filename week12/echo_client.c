#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 100

char escape[] = "exit\n";

void *thread_send(void *arg)
{
    int sockfd = *(int *)arg;
    int size;
    char buf[BUF_SIZE];

    while(1) {
        /*
        printf("Input: \n"); // buffer에 잠깐 넣어놨다가 나중에 실행하는 delay, 다른 일을 먼저 처리함 : "\n"을 꼭 넣어라
                             // 줄 단위로 찍히니까 한줄이 끝날 때까지 기다렸다가 실행을 하는 것
                             // 근데 이렇게 하면 Input 옆에 쓰고 싶은데 Input 아래줄에 쓰게 된다. -> 그럴땐 fflush()를 써라
        */
        printf("Input: "); // Input: 옆에 내용을 쓰기 위해 "\n" 쓰지 않아
        fflush(stdout); // 출력을 안하고 buffer에 쌓아둔 것을 바로 출력하도록 강요 (buffer를 비움)
        // fd 0번이 stdin임 (standard input) --> 키보드 입력을 받음
        // 보통 read 안하고 scanf 쓰겠지만 read 알아두면 좋아
        size = read(0, buf, BUF_SIZE);
        if(size <= 0){
            perror("read");
            exit(1);
        }
        
        buf[size] = '\0';
        
        // 앞에서부터 4번째 글자까지만 비교 (exit 딱 4글자)
        if(strncmp(buf, escape, 4) == 0){
            /* 
            // main thread recv 에서 block이 안풀린 것 (계속 기다리고, 접속을 끊지 못함)
            close(sockfd);  // exit 해도 안끊기고 계속 보내짐 -> 따라서 half close를 해야한다.
            */
           
            // 아 얘가 끊으려고 하는구나 하고 server 쪽에서 반응을 해줌
            // recv에서 block 되어 있던 애가 이제 끊는구나 하고 끊어서 정상종료            
            shutdown(sockfd, SHUT_WR); // write하는 쪽만 닫아    
            break;
        }
        send(sockfd, buf, size, 0);
        printf("Host: %s", buf);
    }
    return NULL;
}

int main(int argc, char *argv[]) 
{
    int sockfd, newfd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int sin_size, len_inet;
    int pid, size;
    char buf[BUF_SIZE];
    pthread_t t_id;

    if (argc < 3){
        fprintf(stderr, "Usage: %s <IP> <PORT> \n", argv[0]);
        return 0;
    }
    // socket을 만들고
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd ==-1) {
        perror("socket");
        return 0;
    }

    // socket 주소를 넣어
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(atoi(argv[2]));
    my_addr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(&(my_addr.sin_zero), 0, 8);

    sin_size = sizeof(struct sockaddr_in);
    
    if(connect(sockfd, (struct sockaddr *)&my_addr, sin_size) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected. (Enter \"exit\" to quit)\n");

    pthread_create(&t_id, NULL, thread_send, &sockfd);
    pthread_detach(t_id);

    while(1) {
        // main thread에서는 상대방 쪽에서 메세지를 받아서 화면에 출력
        // 상대방에게 메세지를 받는 부분만 있으면 됨.
        size = recv(sockfd, buf, BUF_SIZE, 0);  // NULL문자 이전까지만 read
        if (size <= 0) break;
        buf[size] = '\0'; // 문자열 맨 뒤에 NULL 문자 넣어서
        printf("Server: %s", buf); // 개행문자까지 올 예정이라서 굳이 '\n'넣지 않음
    }
    close(sockfd);

    return 0;
}
