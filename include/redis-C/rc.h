#ifndef REDIS_C_RETURN_CODE_H__
#define REDIS_C_RETURN_CODE_H__

// clang-format off

#define REDIS_RC                int

#define REDIS_OK                0

#define REDIS_FAILED_COMMON_BEGIN       -1
#define REDIS_FAILED_COMMON_END         -100

#define REDIS_FAILED_CMS_BEGIN          -101
#define REDIS_FAILED_CMS_END            -150

#define REDIS_ERROR_UNKNOWN             -999

#define REDIS_SUCCESS(rc)                               (rc >= REDIS_OK)
#define REDIS_FAILED(rc)                                (rc < REDIS_FAILED_BEGIN)

#define REDIS_CMD_NULL                                  REDIS_FAILED_COMMON_BEGIN 
#define REDIS_CMD_CONNECTION_FAILED                     REDIS_FAILED_COMMON_BEGIN - 1
#define REDIS_SUB_CMD_NOT_FOUND                         REDIS_FAILED_COMMON_BEGIN - 2

#define REDIS_CMS_SKETCH_EXISTED                        REDIS_FAILED_CMS_BEGIN 



// clang-format on
#endif

