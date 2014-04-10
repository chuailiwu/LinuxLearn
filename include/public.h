
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

#define	MAXLINE	4096			/* max line length */
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef void Sigfunc(int);   /* for signal handlers */
Sigfunc * Signal(int signo, Sigfunc *func);

void err_doit(int errnoflag, int error, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);
void pr_mask(const char *str);
int already_running(const char* lock_file);


#endif


