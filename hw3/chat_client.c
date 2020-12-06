#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUF_SIZE 100

char escape[] = "/q\n";

struct MESSAGE{
        uint16_t len;
        char type;
        char msg[BUF_SIZE];
};

struct USER{
    int sockfd;
    char name[BUF_SIZE];
};

void *thread_send(void *arg)
{
    struct USER user = *(struct USER *)arg;
    struct MESSAGE message;
    int size;
    char buf[BUF_SIZE];

    message.type = 'c';
    strcpy(message.msg, user.name);
    message.len = htons(strlen(message.msg));

    send(user.sockfd, &message, ntohs(message.len) + 3, 0);

    while(1) {
        /*
        printf("Input: \n"); // buffer에 잠깐 넣어놨다가 나중에 실행하는 delay, 다른 일을 먼저 처리함 : "\n"을 꼭 넣어라
                             // 줄 단위로 찍히니까 한줄이 끝날 때까지 기다렸다가 실행을 하는 것
                             // 근데 이렇게 하면 Input 옆에 쓰고 싶은데 Input 아래줄에 쓰게 된다. -> 그럴땐 fflush()를 써라
        */
        // printf("Input: "); // Input: 옆에 내용을 쓰기 위해 "\n" 쓰지 않아
        fflush(stdout); // 출력을 안하고 buffer에 쌓아둔 것을 바로 출력하도록 강요 (buffer를 비움)
        // fd 0번이 stdin임 (standard input) --> 키보드 입력을 받음
        // 보통 read 안하고 scanf 쓰겠지만 read 알아두면 좋아
        size = read(0, buf, BUF_SIZE);
        if(size <= 0){
            perror("read");
            exit(1);
        }
        
        // 앞에서부터 2번째 글자까지만 비교 (/q 딱 2글자)
        if(strncmp(buf, escape, 2) == 0){
            /* 
            // main thread recv 에서 block이 안풀린 것 (계속 기다리고, 접속을 끊지 못함)
            close(sockfd);  // exit 해도 안끊기고 계속 보내짐 -> 따라서 half close를 해야한다.
            */
           
            // 아 얘가 끊으려고 하는구나 하고 server 쪽에서 반응을 해줌
            // recv에서 block 되어 있던 애가 이제 끊는구나 하고 끊어서 정상종료            
            
            message.len = htons(0);
            message.type = 'q';
            send(user.sockfd, &message, 3, 0);
            shutdown(user.sockfd, SHUT_WR); // write하는 쪽만 닫아    
            break;
        }
        else{
            message.type = 'm';
            buf[size] = '\0';
            strcpy(message.msg, buf);
            message.len = htons(strlen(message.msg)-1);
            send(user.sockfd, &message, ntohs(message.len) + 3, 0);
        }
    }
    return NULL;
}


int main(int argc, char *argv[]) 
{
    int sockfd;
    struct sockaddr_in my_addr;
    unsigned int sin_size;
    int size;
    char buf[BUF_SIZE];
    pthread_t t_id;
    struct USER user;

    if (argc < 4){
        fprintf(stderr, "Usage: %s <IP> <PORT> <CLIENT_NAME>\n", argv[0]);
        return 0;
    }

    // socket을 만들고
    user.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(user.sockfd ==-1) {
        perror("socket");
        return 0;
    }

    // socket 주소를 넣어
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[2]));
    my_addr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(&(my_addr.sin_zero), 0, 8);

    strcpy(user.name, argv[3]);

    sin_size = sizeof(struct sockaddr_in);
    
    if(connect(user.sockfd, (struct sockaddr *)&my_addr, sin_size) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected. (Enter \"/q\" to quit)\n");
    printf("Name : %s\n", user.name);

    pthread_create(&t_id, NULL, thread_send, &user);

    pthread_detach(t_id);

    while(1) {
        // main thread에서는 상대방 쪽에서 메세지를 받아서 화면에 출력
        // 상대방에게 메세지를 받는 부분만 있으면 됨.
        size = recv(user.sockfd, buf, BUF_SIZE, 0);  // NULL문자 이전까지만 read

        if (size <= 0) {
            break;
        }
        buf[size]='\0';  // 문자열 맨 뒤에 NULL 문자 넣어서
        printf("%s\n", &buf[3]);
    }
    close(user.sockfd);
    return 0;
}
