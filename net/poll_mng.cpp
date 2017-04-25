#include <signal.h>
#include <sys/syscall.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unisted.h>
#include <typeinfo>
#include "base_socket.h"
#include "peer_socket.h"
#include "common_define.h"
#include "poll_mng.h"


static int init_thread_data(int _max_epoll_size,thread_data_t *_thread_data)
{
	if (!_thread_data)
	{
		return -1;
	}
	_thread_data->epoll_fd = epoll_create(1);
	if (_thread_data->epoll_fd <0)
	{
		ERROR("error = %s",strerror(errno));
		return -1;
	}

	_thread_data->epoll_events_ = NULL;
	_thread_data->epoll_events_ = (struct epoll_event*)malloc(sizeof(struct epoll_event) *_max_epoll_size);

	if (!_thread_data->epoll_events_)
	{
		ERROR();
		return -1;
	}

	memset(_thread_data->epoll_events_,0,sizeof(struct epoll_event) * _max_epoll_size);
	_thread_data->pollmng_run = -1;

	pthread_mutex_init(&_thread_data->poll_mutex_,NULL);
	_thread_data->epoll_sockets_ = NULL;
	


}

