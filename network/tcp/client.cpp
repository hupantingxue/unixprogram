#include "tcp_header.h"

int main(int argc, char **argv) {
    int sockfd = 0;
    struct sockaddr_in servaddr,cliaddr;
    char sendline[1000];
    char recvline[1000];

    if (3 > argc) {
	    printf("Usage: bin ip port\n");
		return 0;
	}

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);
    servaddr.sin_port=htons(32000);

    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    while (fgets(sendline, 10000, stdin) != NULL)
    {
        sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
        n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
        recvline[n]=0;
        fputs(recvline,stdout);
    }

    return 0;
}
