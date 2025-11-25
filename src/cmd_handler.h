#ifndef REDIS_C_CMD_HANDLER_H__
#define REDIS_C_CMD_HANDLER_H__

#include "command/cmd.h"
#include "redis-C/rc.h"

REDIS_RC handle_command(Command* cmd); 

#endif 
