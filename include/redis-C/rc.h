#ifndef REDIS_C_RETURN_CODE_H__
#define REDIS_C_RETURN_CODE_H__

// clang-format off

#define REDIS_RC                int

#define REDIS_OK                0
#define REDIS_FAILED_BEGIN      -1

#define REDIS_SUCCESS(rc)                               (rc >= REDIS_OK)
#define REDIS_FAILED(rc)                                (rc < REDIS_FAILED_BEGIN)

#define REDIS_CMD_NULL                                  REDIS_FAILED_BEGIN 
#define REDIS_CMD_CONNECTION_FAILED                     REDIS_FAILED_BEGIN - 1


// clang-format on
#endif

