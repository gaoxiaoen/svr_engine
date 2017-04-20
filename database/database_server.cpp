#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "cpp_global_func.h"
#include "database_app_server.h"
#include "poll_mng.h"
#include "client_socket.h"
#include "stream_client_socket.h"
#include "game_tlog.h"


#define USE_HIREDIS

static int load_lua_config(const char *_lua_config_file)
{
	lua_State* lua_state = lua_open();
	if (!lua_state) 
		return -1;
	memset(&g_app_info,0,sizeof(g_app_info));
	char process_common_config[MAX_STR_LEN];
	memset(process_common_config,0,sizeof(process_common_config));

	if(luaL_dofile(lua_state,_lua_config_file)!=0 ||
		Lua::GetTable(lua_state,LUA_GLOBALSINDEX,"cxx_private_config")<=0)
	{
		return -1;
	}

	Lua::GetTableNumber(lua_state,-1,"node_id",g_app_info.node_id);
	Lua::GetTableString(lua_state,-1,"redis_ipaddr",g_app_info.redis_client_id);

	lua_pop(lua_state,1);

	lua_close(lua_state);

	return 0;
}


void signal_handle(int _sig)
{
	if ((_sig == SIGINT || _sig == SIGUSR1) && !g_database_app_server->app_pre_stop_)
	{
		g_database_app_server->app_pre_stop_ = true;
	}
}

void main(int argc,char *argv[])
{
	int open_svr = 0;
	char current_dir[MAX_STR_LEN];
	memset(current_dir,0,sizeof(current_dir));
	getcwd(current_dir,sizeof(current_dir));

	char config_filename[MAX_STR_LEN];
	memset(config_filename,0,sizeof(config_filename));

	memset(&g_app_info,0,sizeof(g_app_info));
	if (pre_main(argc,argv,config_filename,&open_svr)<0) 
		return -1;

	if (load_lua_config(config_filename) < 0)
		return -1;
	if (g_app_info.daemon_run)
		daemon_init(".");

	if (g_app_info.daemon_run > 0)
	{
		char full_log_file[MAX_STR_LEN];
		memset(full_log_file,0,sizeof(full_log_file));
		snprintf(full_log_file,sizeof(full_log_file),"%s/%s/%s",current_dir,g_app_info.log_file_dir,g_app_info.process_common_config);
		init_log_file(full_log_file,(log_level_t)g_app_info.min_log_level,1,10,1024*1024,1);
	}

	write_pid(argv[0]);
	adjust_rlimit();
	adjust_signal();

	signal(SIGINT,signal_handle);
	signal(SIGUSR1,signal_handle);
	srand(time(NULL));

	g_poll_mng = new CPollMng(g_app_info.net_thread_num,g_app_info.max_epoll_size,g_app_info.epoll_timeout_ms);
	g_database_app_server = new CDatabaseAppServer(g_app_info.max_epoll_size,g_app_info.epoll_timeout_ms);
	g_database_app_server->load_area_shared_config(g_app_info.area_common_config,&g_app_info);
	g_database_app_server->init_app_server();
	g_database_app_server->debug_app_info();

	g_route_client_mng = new CRouteClientMng(g_app_info.refuse_broadcast,g_app_info.route_num,g_app_info.node_id,APP_TYPE_DATABSE);
	for (int i=0;i<g_app_info.router_num;i++)
	{
		CClientSocket *route_client_socket = new CClientSocket(g_app_info.router_ip_addrs[i],g_app_info.router_ports[i],i,NULL,g_app_info.buffer_size,0);
		route_client_socket->set_base_mng(g_route_client_mng);
		if (route_client_socket->check_socket_connected() <0) 
			exit(-1);

		g_poll_mng->add_socket(route_client_socket);
	}

	g_redis_client_mng = new CRedisClientMng();
	CStreamClientSocket * redis_client_socket = new CStreamClientSocket(g_app_info.redis_ipaddr,g_app_info.redis_port,g_app_info.redis_client_id,
		g_app_info.redis_passwd,g_app_info.buffer_size,0);
	redis_client_socket->set_base_mng(g_redis_client_mng);
	if (redis_client_socket->check_socket_connected()<0 )
		exit(-1);
	g_poll_mng->add_socket(redis_client_socket);

	g_poll_mng->start_poll();
	g_database_app_server->start_app_server();

	g_app_info.app_active = true;
	while(g_app_info.app_active) {
		if (!g_database_app_server->check_runing())
			g_poll_mng->stop_poll();
		if (!g_poll_mng->check_runing())
			g_app_info.app_active = false;
		sleep_ms(1000);
	}


	delete g_poll_mng;
	g_poll_mng = NULL;
	delete g_route_client_mng;
	g_route_client_mng = NULL;

	delete g_redis_client_mng;
	g_redis_client_mng = NULL;

	delete g_database_app_server;
	g_database_app_server = NULL;

	return 0;

}