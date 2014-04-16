
#ifndef	_PUBLIC_H
#define	_PUBLIC_H

#include <sys/types.h>		/* some systems still require this */
#include <sys/stat.h>
#include <sys/termios.h>	/* for winsize */
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif
#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */
#include <signal.h>		/* for SIG_ERR */
#include <errno.h>
#include <stdarg.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>

#define	MAXLINE	4096			/* max line length */
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef void Sigfunc(int);   /* for signal handlers */
Sigfunc * Signal(int signo, Sigfunc *func);

void err_doit(int errnoflag, int error, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);
void pr_mask(const char *str);
int already_running(const char* lock_file);

#define	SA	struct sockaddr
#define	LISTENQ		1024	/* 2nd argument to listen() */

int Accept(int, SA *, socklen_t *);
void Bind(int, const SA *, socklen_t);
void Connect(int, const SA *, socklen_t);
void Listen(int fd, int backlog);

ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);
ssize_t Readline(int fd, void *ptr, size_t maxlen);

#endif


