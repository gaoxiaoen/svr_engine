#ifndef __POLL_MNG_H__
#define __POLL_MNG_H__
#include <phread.h>
#include "base_mng.h"

class CBaseSocket;
class CPollMng;

typedef struct thread_data {
	int thread_idx_;
	CPollMng *poll_mng;
	int pollmng_run;
	int pollmng_tid;
	time_t last_check_idel_tm_;

	int epoll_fd;
	struct epoll_event *epoll_events_;
	pthread_mutex_t poll_mutex;
	CBaseSocket* *epoll_sockets_;
	list_head  reconnect_list_;
} thread_data_t;

class CPollMng
{
public:
	CPollMng(int _thread_num, int _epoll_size,int _epoll_timeout_ms);
	~CPollMng();

public:
	void start_poll();
	void stop_epoll();
	bool check_running();
	int add_socket(CBaseSocket *_base_socket);
	int remove_socket(long long _socket_id,int _socket_fd);

	int pop_socket_msg(long long _socket_id,int _socket_fd,char *_buff,int _buff_size);
	int push_socket_msg(long long _socket_id,int _socket_fd,char *_buff,int _buff_size);
	int set_socket_not_reconnect(long long _socket_id,int _client_index);

private:
	int modify_socket(CBaseSocket *_base_socket);
	static void* process_sockets(void *_pollmng);

	int epoll_size_;
	int epoll_timeout_ms_;
	int poll_started_;
	int thread_num_;
	thread_data_t *thread_data_;
}

extern CPollMng *g_poll_mng;

#endif