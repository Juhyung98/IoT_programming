#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main() 
{
    int s;
    struct sockaddr_in server_addr;
    int addr_len;
    char msg[1001];
    int len;
    int read_len;

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

    int fd;
    fd = open("test.c", O_WRONLY | O_CREAT, 0644);  // 0644: read, write 권한 주는 것
    while(1) {
        len = read(s, msg, 1000);
        if (len == -1) {
            perror("read");
            close(s);
            return 0;
        }
        if (len == 0) {
            printf("Disconnected\n");
            break;
        }

        // msg[len] = 0;
        // printf("%s", msg);
        write(1, msg, len); // stdout 이 1 : fd 자리에 1 넣으면 됨(화면에 출력)
        write(fd, msg, len); // stdout 이 1 : fd 자리에 1 넣으면 됨(화면에 출력)
    }
    
    close(fd);
    close(s);

    return 0;
}
