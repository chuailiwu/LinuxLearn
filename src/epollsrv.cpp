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

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10


/* 将文件描述符设置成非阻塞的*/
int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

/* 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数enable_et指定是否对fd启用ET模式 */
void addfd(int epollfd, int fd, bool enable_et)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if (enable_et)
	{
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

/* LT模式工作流程 */
void lt(epoll_event* events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for (int i = 0; i < number; ++i)
	{
		int sockfd = events[i].data.fd;
		if (sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, false);
		} else if (events[i].events & EPOLLIN)
		{
			/* 只要socket读缓存中还有未读出的数据，这段代码就被触发 */
			printf("level trigger ...\n");
			memset(buf, 0, BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
			if (ret <= 0)
			{
				close(sockfd);
				continue;
			}
			printf("get %d bytes of content: %s\n", ret, buf);
		} else
		{
			printf("something else happened\n");
		}
	}
}

/* ET模式工作流程 */
void et(epoll_event* events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for (int i = 0; i < number; ++i)
	{
		int sockfd = events[i].data.fd;
		if (sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, true);
		} else if (events[i].events & EPOLLIN)
		{
			/* 这段代码不会被重复触发，所以我们循环读取数据，以确保把socket读缓存中的所有数据读出 */
			printf("edge trigger ...\n");
			while (1)
			{
				memset(buf, 0, BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE -1, 0);
				printf("ret:%d \n",ret);
				if (ret < 0)
				{
					// printf("EAGAIN:%d, EWOULDBLOCK:%d\n", EAGAIN, EWOULDBLOCK);
					// 对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕。此后，epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
					{
						printf("read later\n");
						break;
					} else
					{
						printf("%s\n", strerror(errno));
					}
					close(sockfd);
					break;
				} else if (ret == 0)
				{
					close(sockfd);
					printf("close sockfd\n");
					break;
				} else
				{
					printf("get %d bytes of content: %s\n", ret, buf);
				}
			}
		} else
		{
			printf("something else happened\n");
		}
	}
}

void err_sys(const char* str)
{
	char buf[1024];
	snprintf(buf, sizeof(buf), "%s %s", str, strerror(errno));
	exit(-1);
}
// 测试LT模式与ET模式
int test_lt_et(int argc, char* argv[])
{
	if (argc <= 2)
	{
		printf("usage: %s ip_address port_number\n", basename(argv[0]));
		return 1;
	}
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	
	if (bind(listenfd, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s", strerror(errno));
		printf("%d:%s\n", __LINE__, buf);
		return -1;
	}
	
	if (listen(listenfd, 5) == -1)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s", strerror(errno));
		printf("%d:%s\n", __LINE__, buf);
		return -1;
	}
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd, true);
	
	while (1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			printf("epoll failure\n");
			break;
		}
		//lt(events, ret, epollfd, listenfd); /* 使用LT模式 */
		 et(events, ret, epollfd, listenfd); /* 使用ET模式 */
	}
	close(listenfd);
	return 0;
}




struct fds
{
	int epollfd;
	int sockfd;
};

/* 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数oneshot指定是否注册fd上的EPOLLONESHOT事件 */
void addfd_oneshot(int epollfd, int fd, bool oneshot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if (oneshot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}
// 重置fd上的事件，这样操作之后，尽管fd上的EPOLLONESHOT事件被注册，但是操作系统仍然会触发fd上的EPOLLIN事件，且只触发一次
void reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

// 工作线程
void* worker(void* arg)
{
	int sockfd = ((fds*)arg)->sockfd;
	int epollfd = ((fds*)arg)->epollfd;
	printf("start new thread to receive data on fd:%d\n",sockfd);
	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);
	// 循环读取sockfd上的数据，直到遇到EAGAIN错误
	while (1)
	{
		int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
		if (ret == 0)
		{
			close(sockfd);
			printf("foreiner closed the connection\n");
			break;
		} else if (ret < 0)
		{
			if (errno == EAGAIN)
			{
				printf("read later\n");
				// 休眠5s，模拟数据处理过程
				sleep(30);
				reset_oneshot(epollfd, sockfd);
				break;
			}
		} else
		{
			printf("get content:%s\n", buf);
		}
	}
	printf("end thread receiving data on fd:%d\n", sockfd);
	return 0;
}

// 测试多线程下EPOLLONESHOT
int test_one_shot(int argc, char* argv[])
{
	if (argc <= 2)
	{
		printf("usage: %s ip_address port_number\n", basename(argv[0]));
		return 1;
	}
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	
	int opt = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s", strerror(errno));
		printf("%d:%s\n", __LINE__, buf);
		return -1;
	}
	
	if (bind(listenfd, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s", strerror(errno));
		printf("%d:%s\n", __LINE__, buf);
		return -1;
	}
	
	if (listen(listenfd, 5) == -1)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s", strerror(errno));
		printf("%d:%s\n", __LINE__, buf);
		return -1;
	}
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	/*
	注意，监听socket listenfd上是不能注册EPOLLONESHOT事件的，否则应用程序只能处理一个客户连接！
	因为后续的客户连接请求将不再触发listenfd上的EPOLLIN事件
	*/
	addfd_oneshot(epollfd, listenfd, false);
	
	while (1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			printf("epoll failure\n");
			break;
		}
		for (int i = 0; i < ret; ++i)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == listenfd)
			{
				struct sockaddr_in client_address;
				socklen_t client_addrlength = sizeof(client_address);
				int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
				// 对每个非监听文件描述符都注册EPOLLONESHOT事件
				addfd_oneshot(epollfd, connfd, true);
			} else if (events[i].events & EPOLLIN)
			{
				pthread_t thread;
				fds fds_for_new_worker;
				fds_for_new_worker.epollfd = epollfd;
				fds_for_new_worker.sockfd = sockfd;
				// 新启动一个工作线程为sockfd服务
				pthread_create(&thread, NULL, worker, (void*)&fds_for_new_worker);
			} else
			{
				printf("something else happened\n");
			}
		}
	}
	close(listenfd);
	return 0;
}


int main(int argc, char* argv[])
{
//	test_lt_et(argc, argv);
	test_one_shot(argc, argv);
}


