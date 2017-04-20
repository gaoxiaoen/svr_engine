#include "redis_client_mng.h"

CRedisClientMng::CRedisClientMng():CBaseMng()
{
	memset(redis_clients_,0,sizeof(redis_clients_));
}

CRedisClientMng::~CRedisClientMng()
{
	for(int i=0;i<MAX_REDIS_NODE_NUM;i++)
	{
		if(redis_clients_[i])
		{
			redis_clients_[i]->disconnect_net();
			delete redis_clients_[i];
			redis_clients_[i] = NULL;
		}
	}
}

int CRedisClientMng::new_mng_object(long long _socket_id,int _socket_fd,int _socket_eventfd,
	int _identity_id,char *_redis_auth_pwd,int _buffer_size)
{
	if(_socket_id <0 || _socket_fd < 0 || _socket_eventfd <0 || _identity_id <0 ||
		_identity_id > MAX_REDIS_NODE_NUM || !redis_passwd)
		return -1;

	CRedisClient * redis_client = new CRedisClient(_socket_id,_socket_fd,_socket_eventfd,_identity_id,
		_redis_auth_pwd,_buffer_size);

	if (!redis_client)
		return -1;
	pthread_mutex_lock(&new_mng_mutex);
	list_add_tail(&redis_client->link_,&new_list_);
	pthread_mutex_unlock(&new_mng_mutex);

	return 0;
}

int CRedisClient::free_mng_object(long long _socket_id,int _socket_eventfd,int _extra_data)
{
	if(_socket_id<0 || _socket_eventfd <0 || _extra_data <0 || _extra_data >= MAX_REDIS_NODE_NUM)
		return -1;

	free_object_t * free_obj = (free_object_t)malloc(sizeof(free_object_t));

	if (!free_obj)
	{
		return -1;
	}

	free_obj->socket_id = _socket_id;
	free_obj->socket_eventfd = _socket_eventfd;
	free_obj->extra_data = _extra_data;

	pthread_mutex_lock(&free_mng_object);
	list_add_tail(&free_obj->link_,&free_list_);
	pthread_mutex_unlock(&free_mng_object);

	return 0;
}


void CRedisClientMng:check_redis_obuf()
{
	int idx = 0;
	do{
		CRedisClient * redis_client=redis_clients_[idx++];
		if (redis_client && redis_client->get_redis_status()==REDIS_COMMAND_STATUS)
		{
			redisContext *c = &(redis_client->get_redis_ctx()->c);
			if(sdslen(c->obuf)<=0)
				return;
			long long socket_id = redis_client->get_socket_id();
			int socket_fd = redis_client->get_redis_fd();
			int send_len = g_poll_mng->push_socket_msg(socket_id,socket_fd,c->obuf,sdslen(c->obuf));
			if(send_len >0)
				sdsrange(c->obuf,send_len,-1);
		}
	}while (idx < MAX_REDIS_NODE_NUM);
}

int CRedisClientMng::app_server_process(CAppServer *_app_server)
{
	if(!_app_server)
		return -1;
	list_head process_poll_list;
	INIT_LIST_HEAD(&process_poll_list);

	pthread_mutex_lock(&new_mng_mutex);
	list_swap(&new_list_,&process_poll_list);
	pthread_mutex_unlock(&new_mng_mutex);

	{
		list_head *client_pos = NULL;
		list_for_each_safe(client_pos,&process_poll_list){
			CRedisClient *redis_client = list_entry(client_pos,CRedisClient,link_);
			int identity_id = redis_client->get_identity();
			list_del(&redis_client->link_);

			if (redis_clients_[identity_id] || _app_server->add_net_object(redis_client)!=0)
			{
				redis_client->disconnect_net();
				delete redis_client;
				continue;
			}
			redis_clients_[identity_id] = redis_client;
		}
	}

	check_redis_obuf()

	INIT_LIST_HEAD(&process_poll_list);
	pthread_mutex_unlock(&free_mng_mutex_);
	list_swap(&free_list_,&process_poll_list);
	pthread_mutex_unlock(&free_list_);

	{
		list_head * free_pos = NULL;
		list_for_each_safe(free_pos,&process_poll_list){
			free_object_t *free_obj = list_entry(free_pos,free_object_t,link_);
			list_del(&free_obj->link_);
			long long socket_id = free_obj->socket_id;
			int socket_eventfd = free_obj->socket_eventfd;
			int identity_id = free_object->extra_data;
			free(free_obj);
			free_obj = NULL;

			CRedisClient *redis_client = redis_clients_[identity_id];
			if(!redis_client)
				continue;
			if(redis_client && redis_client->check_equal(socket_id,socket_eventfd)!=0)
				continue;

			delete redis_client;
			redis_clients_[identity_id] = NULL;
		}
	}

	return 0;
}

CRedisClientMng* CRedisClientMng::get_redis_client(int _node_id)
{
	if(_node_id <0 || node_id >= MAX_REDIS_NODE_NUM)
		return NULL;
	return redis_clients_[_node_id];
}


CRedisClientMng *g_redis_client_mng = NULL;