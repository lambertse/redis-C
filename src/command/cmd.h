#ifndef COMMAND_TYPE_H__
#define COMMAND_TYPE_H__

#include <stdlib.h>
typedef enum {
    CMD_PING = 0,
    CMD_STRING,
    CMD_SORTED_SET,
    CMD_SET,
    CMD_GEOSPATIAL,
    CMD_BLOOM_FILTER,
    CMD_CMS
} CommandType;

typedef struct {
    CommandType type;
    int sub_cmd; 
    char** arg;
} Command;

static Command* create_command(CommandType type, int sub_cmd, char** arg){
    Command* cmd = malloc(sizeof(Command));

    cmd->type = type;
    cmd->sub_cmd = sub_cmd;
    cmd->arg = arg;

    return cmd;
}

#endif
