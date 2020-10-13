#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    char addr_str1[] = "192.168.0.10";
    char addr_str2[] = "192.168.0.255";
    unsigned int addr1, addr2;
    addr1 = inet_addr(addr_str1);
    if (addr1 == INADDR_NONE)
        printf("addr1 : Error occured\n");
    else
        printf("Network ordered address 1 : 0x%08x\n", ntohl(addr1));
    addr2 = inet_addr(addr_str2);
    if (addr2 == INADDR_NONE)
        printf("addr2 : Error occured\n");
    else
        printf("Network ordered address 2 : 0x%08x\n", ntohl(addr2));
    return 0;
}