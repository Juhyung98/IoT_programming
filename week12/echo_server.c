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

char escape[] = "exit\n";
int num_client = 0;
pthread_mutex_t mutex;

void *thread_main(void *arg)
{
    int size;
    int s = *(int *)arg;
    char buf[BUF_SIZE];
    free(arg);
    // 오류가 나거나 접속이 끊겼을 때 while문 탈출
    while(1){
        if((size = recv(s, buf, BUF_SIZE, 0)) > 0) {
            buf[size] = '\0';

            // print message
            printf("Recv: %s\n", buf);
            send(s, buf, size, 0);
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
    printf("Disconnected: %d\n", num_client);

    close(s);
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
    int *arg;

    pthread_mutex_init(&mutex, NULL);

    if (argc < 2){
        fprintf(stderr, "Usage: %s <IP> <PORT> \n", argv[0]);
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
            perror("accept");
            return 0;
        }

        printf("Connected: %s %d num_client : %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), num_client);
        
        // critical section에 들어가니까 lock 거는 것
        pthread_mutex_lock(&mutex);
        // 접속이 끝났으면
        num_client++;
        pthread_mutex_unlock(&mutex);

        arg = (int *)malloc(sizeof(int));
        *arg = newfd;
        pthread_create(&t_id, NULL, thread_main, arg);
        pthread_detach(t_id);
    }

    close(sockfd);
    return 0;
}
