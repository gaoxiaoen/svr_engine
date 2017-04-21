#include <unistd.h>
#include <math.h>
#include <float.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/select.h>

#include "common_define.h"
#include "log.h"
#include "lua_server.h"
#include "cpp_global_func.h"

#define LUA_TRUE "true"
#define LUA_FALSE "false"


void daemon_init(const char *_deamon_wdir)
{
	pid_t pid;
	signal(SIGCHLD,SIG_IGN);
	if((pid ==fork())){
		exit(0);    //父进程退出
	}
	else if(pid <0)
	{
		printf("fork1 failed(%s)\n",strerror(errno));
	}
	setsid();
	if ((pid = fork()))
	{
		exit(0);
	}
	else if(pid <0)
	{
		printf("fork2 failed(%s)\n",strerror(errno));
	}

	for(int i=0;i<getdtablesize();i++)
		close(i);
	umask(0);

	int std_fd = open("dev/null",O_RDWR);
	dup2(std_fd,STDERR_FILENO);
	dup2(std_fd,STDIN_FILENO);
	dup2(std_fd,STDOUT_FILENO);
}