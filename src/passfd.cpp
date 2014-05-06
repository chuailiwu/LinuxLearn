#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define	_SOCKADDR_LEN  // 这是专为 alpha 平台设置
#include <sys/socket.h>
// 某些平台没有这个宏，需要自己定义
#ifndef CMSG_LEN
#define	CMSG_LEN( a )	( sizeof(struct cmsghdr) + a )
#endif
/*
描述符是OS为每个进程维护的，其实是描述符表的入口，里边有复杂的数据结构以屏蔽文件、设备、socket等的不同。
描述符是进程内唯一的，出了该进程就没有了意义。比如都是描述符5，对进程A和进程B其含义是完全不同的，因此需要OS参与才能完成描述符传递。
unix 系统中有两个办法来完成这个任务： 
BSD   sendmsg, recvmsg 方法
SYSV  ioctl 方法 在外观上更专业一点， 宏定义 I_SEDNFD 和 I_RECVFD是专为传递描述符而设置
*/

// 另一种方法，msghdr和cmsghdr填充方式有所不同
int pass_fd_2()
{
	int	rv, i;
	int	sp[2];
	int	ofd;

	ofd = open("test.txt", O_RDWR | O_CREAT, 0666);
	if (ofd == -1)
	{
		perror("open file");
		exit(1);
	}

	rv = socketpair(AF_UNIX, SOCK_STREAM, 0, sp);
	if (rv != 0)
	{
		perror("socket pair");
		exit(1);
	}
	printf("ofd=%d, sp-1=%d, sp-2=%d\n", ofd, sp[0], sp[1]);
	{
		struct msghdr msg;
		struct iovec iov;
		struct cmsghdr *cmsg;
		char	buf[500];
		char	b = ' ';

		memset(buf, 0x00, sizeof(buf));
		memset(&msg, 0x00, sizeof(struct msghdr));
		// ID1	在linux 中这是必须的.其他可以忽略
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		iov.iov_base = &b;
		iov.iov_len = 1;
		// ID1
		msg.msg_control = buf;
		msg.msg_controllen = sizeof(buf);
		cmsg = CMSG_FIRSTHDR(&msg);
		if (cmsg) // 这里必须做检查；看 REF0001
		{
			int	*pfd;
			int	fd[2] = { 0, 0 };
			fd[0] = ofd;
#ifndef SOCKETSYS // 看程序后说明 BUG0001
			cmsg->cmsg_level = SOL_SOCKET;
			cmsg->cmsg_type  = SCM_RIGHTS;
			cmsg->cmsg_len   = CMSG_LEN(sizeof(int) * 2);
#endif
			pfd = (int *)CMSG_DATA(cmsg);
			memcpy(pfd, fd, sizeof(int) * 2);
			msg.msg_controllen = CMSG_LEN(sizeof(int) * 2);
			rv = sendmsg(sp[0], &msg, 0);
			if (rv == -1)
			{
				perror("send msg");
				exit(1);
			}
			printf("send len=%zu rv=%d\n", msg.msg_controllen, rv);
		}
		else
		{
			printf("point is null\n");
		}
	}
	{
		struct msghdr rmsg;
		struct iovec iov;
		struct cmsghdr *rcm;
		char	buf[100];
		char	b;

		memset(buf, 0x00, sizeof(buf));
		memset(&rmsg, 0x00, sizeof(struct msghdr));
		// ID1	在linux 中这是必须的.其他可以忽略
		rmsg.msg_iov = &iov;
		rmsg.msg_iovlen = 1;
		iov.iov_base = &b;
		iov.iov_len = 1;
		// ID1
		rmsg.msg_control = buf;
		rmsg.msg_controllen = sizeof(buf);
		rv = recvmsg(sp[1], &rmsg, 0);
		if (rv == -1)
		{
			perror("recv error ");
			exit(1);
		}
		printf("recv rv=%d len=%zu\n", rv, rmsg.msg_controllen);
		rcm = CMSG_FIRSTHDR(&rmsg);
		if (rcm)
		{
			int	*rfd;
			printf("cmsg->len = %zu\n", rcm->cmsg_len);
			printf("cmsg->type = %d\n", rcm->cmsg_type);
			printf("cmsg->level = %d\n", rcm->cmsg_level);
			rfd = (int *)CMSG_DATA(rcm);
			printf("recv fd=%d\n", *rfd);
			write(*rfd, "hello world", 11);
		}
		else
		{
			printf("recv data invalid\n");
		}
	}
	return 0;
}



// Linux高性能服务器编程源码
static const int CONTROL_LEN = CMSG_LEN(sizeof(int));

void send_fd(int fd, int fd_to_send)
{
	struct iovec iov[1];
	struct msghdr msg;
	char buf[0];

	iov[0].iov_base = buf;
	iov[0].iov_len = 1;
	msg.msg_name    = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov     = iov;
	msg.msg_iovlen = 1;

	cmsghdr cm;
	cm.cmsg_len = CONTROL_LEN;
	cm.cmsg_level = SOL_SOCKET;
	cm.cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(&cm) = fd_to_send;
	msg.msg_control = &cm;
	msg.msg_controllen = CONTROL_LEN;

	sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
	struct iovec iov[1];
	struct msghdr msg;
	char buf[0];

	iov[0].iov_base = buf;
	iov[0].iov_len = 1;
	msg.msg_name    = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov     = iov;
	msg.msg_iovlen = 1;

	cmsghdr cm;
	msg.msg_control = &cm;
	msg.msg_controllen = CONTROL_LEN;

	recvmsg(fd, &msg, 0);

	int fd_to_read = *(int *)CMSG_DATA(&cm);
	return fd_to_read;
}

int main(int argc, char *argv[])
{
	int pipefd[2];
	int fd_to_pass = 0;

	int ret = socketpair(PF_UNIX, SOCK_DGRAM, 0, pipefd);
	assert(ret != -1);

	printf("pipefd[0]=%d, pipefd[1]=%d\n", pipefd[0], pipefd[1]);
	pid_t pid = fork();
	assert(pid >= 0);

	if (pid == 0)
	{
		close(pipefd[0]);
		fd_to_pass = open("test.txt", O_RDWR | O_CREAT, 0666);
		printf("child process... fd=%d, pipefd[1]=%d\n", fd_to_pass, pipefd[1]);
		send_fd(pipefd[1], (fd_to_pass > 0) ? fd_to_pass : 0);
		close(fd_to_pass);
		exit(0);
	}

	close(pipefd[1]);
	fd_to_pass = recv_fd(pipefd[0]);
	printf("parent process... fd=%d, pipefd[0]=%d\n", fd_to_pass, pipefd[0]);
	char buf[1024];
	memset(buf, '\0', 1024);
	read(fd_to_pass, buf, 1024);
	printf("I got fd %d and data %s\n", fd_to_pass, buf);
	close(fd_to_pass);
}

