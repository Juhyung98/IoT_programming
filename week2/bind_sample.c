#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in my_addr;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);

    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(23456);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(my_addr.sin_zero), 0 ,8);

    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) == -1){
        //printf("%d\n", errno);
        perror("Bind : ");
        return 0;
    }

    // system("netstat -pa --tcp");
    close(sockfd);
    return 0;
}