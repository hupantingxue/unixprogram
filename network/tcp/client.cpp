#include "tcp_header.h"

int main(int argc, char **argv) {
	int sockfd = 0;

    if (3 > argc) {
	    printf("Usage: bin ip port\n");
		return 0;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    return 0;
}
