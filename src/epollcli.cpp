#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
	int sockfd, n;
	struct sockaddr_in servaddr;
	
	if (argc != 3)
	{
		printf("usage: a.out <IPaddress> <Port>\n");
		return -1;
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error\n");
		return -1;
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		printf("inet_pton error for %s\n", argv[1]);
		return -1;
	}
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("connect error\n");
		return -1;
	}
	char buf[1024];
	while (fgets(buf, 1024, stdin) != 0)
	{
		write(sockfd, buf, strlen(buf));
	}
	exit(0);
}

