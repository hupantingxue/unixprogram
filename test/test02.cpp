#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int ip = 0;
    if (2 > argc) {
        printf("Usage: %s <ip>\n", argv[0]);
        return -1;
    }

#if 0
    struct sockaddr_in stInAddr;
    stInAddr.sin_addr.s_addr = ip;
    printf("Convert to ip[%s]\n", inet_ntoa(stInAddr.sin_addr));
#endif
    printf("Convert to ip[%u]\n", inet_addr(argv[1]));
    return 0;
}
