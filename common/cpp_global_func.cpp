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


