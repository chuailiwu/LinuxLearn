
#include "public.h"

void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char	buf[MAXLINE];

	vsnprintf(buf, MAXLINE, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s",
		  strerror(error));
	strcat(buf, "\n");
	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(NULL);		/* flushes all stdio output streams */
}

void err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_quit(const char *fmt, ...)
{
        va_list         ap;

        va_start(ap, fmt);
        err_doit(0, 0, fmt, ap);
        va_end(ap);
        exit(1);
}

void pr_mask(const char *str)
{
        sigset_t        sigset;
        int                     errno_save;

        errno_save = errno;             /* we can be called by signal handlers */
        if (sigprocmask(0, NULL, &sigset) < 0)
                err_sys("sigprocmask error");

        printf("%s", str);
        if (sigismember(&sigset, SIGINT))   printf("SIGINT ");
        if (sigismember(&sigset, SIGQUIT))  printf("SIGQUIT ");
        if (sigismember(&sigset, SIGUSR1))  printf("SIGUSR1 ");
        if (sigismember(&sigset, SIGALRM))  printf("SIGALRM ");

        /* remaining signals can go here  */

        printf("\n");
        errno = errno_save;
}

Sigfunc * Signal(int signo, Sigfunc *func)
{
        struct sigaction        act, oact;

        act.sa_handler = func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        if (signo == SIGALRM) {
#ifdef  SA_INTERRUPT
                act.sa_flags |= SA_INTERRUPT;
#endif
        } else {
#ifdef  SA_RESTART
                act.sa_flags |= SA_RESTART;
#endif
        }
        if (sigaction(signo, &act, &oact) < 0)
                return(SIG_ERR);
        return(oact.sa_handler);
}

