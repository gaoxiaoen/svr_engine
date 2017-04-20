#ifndef __REDIS_CLIENT_MNG_H
#define __REDIS_CLIENT_MNG_H

#include "common_define.h"
#include "base_mng.h"
#include "redis_client.h"
#include "app_server.h"
#include "poll_mng.h"
#include "log.h"
#include "lua_server.h"

class CRedisClientMng:public CBaseMng
{
public:
	CRedisClientMng();
	~CRedisClientMng();

public:
	virtual int new_mng_object(long long _socket_id,int _socket_fd,int _socket_eventfd,int _extra_data0,
		char *_extra_data1,int _buffer_size);
	virtual int free_mng_object(long long _socket_id,int _socket_eventfd,int _extra_data);

	int app_server_process(CAppServer *_app_server);
	int get_redis_client(int _node_id);
	void check_redis_obuf();

private:
	CRedisClient *redis_clients_[MAX_REDIS_NODE_NUM];
};

extern CRedisClientMng *g_redis_client_mng;

#endif