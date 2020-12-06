#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#define BUF_SIZE 100

char escape[] = "/q\n";
int num_client = 0;
pthread_mutex_t mutex;
int max_idx = 0;

struct MESSAGE{
        uint16_t len;
        char type;
        char msg[BUF_SIZE];
};

struct USER {
    int sockfd;
    int port;
    char ip[BUF_SIZE];
    char name[BUF_SIZE];
};


struct USER client[BUF_SIZE];

void *thread_main(void *arg)
{
    int size;
    int s = *(int *)arg;
    char buf[BUF_SIZE];
    char server_msg[BUF_SIZE];
    char chat_msg[BUF_SIZE];
    char type;

    struct MESSAGE message;
    
    // 오류가 나거나 접속이 끊겼을 때 while문 탈출

    while(1){
        if((size = recv(s, buf, BUF_SIZE, 0)) > 0) {
            buf[size]='\0';
            printf("GET: %s\n", &buf[3]);
            type = buf[2];
            switch (type)
            {
            case 'c':
                buf[size]='\0';
                strcpy(client[s-4].name, &buf[3]);
                max_idx++;
                
                printf("name: %s\n", client[s-4].name);
                printf("ip: %s\n", client[s-4].ip);
                printf("port: %d\n", client[s-4].port);
                printf("socket: %d\n", s);
                printf("Users: %d\n", num_client);
                break;
            case 'm':
                message.type = 'm';
                buf[size]='\0';

                strcpy(server_msg, &buf[3]);
                strcpy(chat_msg, client[s-4].name);
                strcat(chat_msg, ": ");
                strcat(chat_msg, server_msg);
                strcpy(message.msg, chat_msg);
                
                message.len = htons(strlen(message.msg));
                for (int j = 0; j <= max_idx; j++){
                    if (client[j].sockfd == 0){
                        continue;
                    }
                    send(client[j].sockfd, &message, ntohs(message.len)+3, 0);
                }
                break;
            case 'q':
                
                client[s-4].sockfd = 0;
                client[s-4].ip[0] = '\0';
                client[s-4].name[0] = '\0';

            default:
                break;
            }
        }
        // 값이 0 이면 접속이 끊긴 것.
        else {
            break;
        }
    }
    // critical section에 들어가니까 lock 거는 것
    pthread_mutex_lock(&mutex);
    // 접속이 끊길 때
    num_client--;
    pthread_mutex_unlock(&mutex);
    printf("Users: %d\n", num_client);
    printf("Disconnected\n");

    close(s);
    return NULL;
}

int main(int argc, char *argv[]) 
{
    int sockfd, newfd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int sin_size;
    char buf[BUF_SIZE];
    pthread_t t_id;
    int *arg;
    int idx;

    pthread_mutex_init(&mutex, NULL);

    if (argc < 2){
        fprintf(stderr, "Usage: %s <PORT> \n", argv[0]);
        return 0;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket");
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[1]));
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(my_addr.sin_zero), 0, 8);

    // bind하고
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");                 
        return 0;
    }

    // port열었어요
    if(listen(sockfd, 5) < 0) {
        perror("listen");
        return 0;
    }

    sin_size = sizeof(struct sockaddr_in);

    printf("waiting....\n");

    while(1) {
        
        // 접속이 오면 accept되고 새로운 소켓이 나와요
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd < 0){
            switch(errno) {
                case EINTR:
                    printf("Interrupt\n");
                    continue;
                default:
                    perror("accept");
            }
            return 0;
        }

        // critical section에 들어가니까 lock 거는 것
        pthread_mutex_lock(&mutex);
        // 접속이 끝났으면
        num_client++;
        pthread_mutex_unlock(&mutex);


        
        
        idx = newfd - 4;
        client[idx].sockfd = newfd;
        
        strcpy(client[idx].ip, inet_ntoa(their_addr.sin_addr));
        
        client[idx].port = ntohs(their_addr.sin_port);
        printf("Connected!\n");
        printf("%s:%d\n", client[idx].ip, client[idx].port);

        pthread_create(&t_id, NULL, thread_main, &client[idx].sockfd);
        pthread_detach(t_id);
        
    }

    close(sockfd);
    return 0;
}
