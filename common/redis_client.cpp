#include "common_define.h"
#include "poll_mng.h"
#include "log.h"
#include "redis_client.h"
#include <sys/socket.h>
#include "base_socket.h"

CRedisClient::CRedisClient(long long _socket_id,int _socket_fd,int _socket_eventfd,int _client_identity,char * _redis_auth_pwd,int _buffer_size):
	CNetObject(_socket_id,_socket_fd,_socket_eventfd,_buffer_size)
{
	identity_id_ = _client_identity;
	memset(redis_auth_pwd_,0,sizeof(redis_auth_pwd_));
	if(_redis_auth_pwd)
		snprintf(redis_auth_pwd_,sizeof(redis_auth_pwd_),"%s",_redis_auth_pwd);
	create_redis_ctx();
}

CRedisClient::~CRedisClient()
{
	if (redis_ctx_)
	{
		redisAsyncFree(redis_ctx_);
		redis_ctx_ = NULL;
		redis_status = REDIS_NEW_STATUS;
	}
}

int CRedisClient::handle_package()
{
	if(!g_poll_mng)
		return -1;

	int package_len = g_poll_mng->pop_socket_msg(socket_id_,socket_fd_,recv_pkg_buffer_,buffer_size_);

	if (package_len <= 0)
		return package_len;

	redisContext *c = &redis_ctx_->c;

	if (redisReaderFeed(c->reader,recv_pkg_buffer_,package_len) != REDIS_OK)
	{
		__redisSetError(c,c->reader->err,c->reader->errstr);
		ERROR("");
		return -1;
	}

	redisProcessCallbacks(redis_ctx_);

	return 0;
}
