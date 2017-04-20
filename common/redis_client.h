#ifndef __REDIS_CLIENT_H__
#define __REDIS_CLIENT_H__

#include <time.h>
#include "net_object.h"
#include "hiredis.s"
#include "async.h"
extern "C" {
	#include "sds.h"
}


typedef enum{
	REDIS_NEW_STATUS = 0,
	REIDS_AUTHING_STATUS = 1,
	REDIS_COMMAND_STATUS = 2,
	REDIS_MAX_STATUS = 3
} redis_status_t;

extern void redis_ack_cb(redisAsyncContext *_redis_ctx,void *_reply,void *_priv);
extern void debug_redis_ack(redisAsyncContext *_redis_ctx,void *_reply,void *_priv);
extern "C" redisContext *redisContextInit(void);
extern "C" redisAsyncContext * redisAsyncInitialize(redisContext *c);
extern "C" int __redisAsyncCommand(redisAsyncContext *ac,redisCallbackFn *fn,void *privdata,char *cmd,size_t len);
extern "C" void __redisSetError(redisContext *c,int type,const char *str);
extern "C" void redisProcessCallbacks(redisAsyncContext *ac);

class CRedisClient:public CNetObject
{
public:
	CRedisClient(long long _socket_id,int _socket_fd,int _socket_eventfd,int _client_identity,
		char *_redis_auth_pwd,int _buffer_size);
	~CRedisClient();

public:
	virtual int handle_package();
	int get_identity()
	{
		return identity_id_;
	}

	int check_equal(long long _socket_id,int socket_eventfd);

	int hiredisAsyncCommand(redisCallbackFn *fn,void *privdata,const char *format, ...);
	int hiredisAsyncRawRedisCommand(redisCallbackFn *fn,void *privdata,char *raw_cmd,int raw_cmd_len);

	redisAsyncContext* get_redis_ctx()
	{
		return redis_ctx_;
	}

	void set_redis_status(redis_status_t _status)
	{
		redis_status_ = _status;
	}

	redis_status_t get_redis_status()
	{
		return redis_status_;
	}

	void test_send_package();

private:
	void create_redis_ctx();

private:
	int identity_id_;
	redisAsyncContext* redis_ctx_;
	redis_status_t redis_status_;
	char redis_auth_pwd_[MAX_STR_LEN];
}

#endif