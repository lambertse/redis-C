/*
 * @file lwlog.h
 * @auther Akagi201
 *
 * @date 2014/11/30
 */

#ifndef LWLOG_H_
#define LWLOG_H_ 

#include <stdio.h>
#include <errno.h>
#include <string.h>


#ifndef LOG_LEVEL
#define LOG_LEVEL (6)
#endif

#ifndef LOG_COLOR
#define LOG_COLOR (7)
#endif

// log levels the same as syslog
#define EMERG (0)
#define ALERT (1)
#define CRIT (2)
#define ERR (3)
#define WARNING (4)
#define NOTICE (5)
#define INFO (6)
#define DEBUG (7)

// colors
#define NONE                 "\e[0m"
#define BLACK                "\e[0;30m"
#define L_BLACK              "\e[1;30m"
#define RED                  "\e[0;31m"
#define L_RED                "\e[1;31m"
#define GREEN                "\e[0;32m"
#define L_GREEN              "\e[1;32m"
#define BROWN                "\e[0;33m"
#define YELLOW               "\e[1;33m"
#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
#define PURPLE               "\e[0;35m"
#define L_PURPLE             "\e[1;35m"
#define CYAN                 "\e[0;36m"
#define L_CYAN               "\e[1;36m"
#define GRAY                 "\e[0;37m"
#define WHITE                "\e[1;37m"

#define BOLD                 "\e[1m"
#define UNDERLINE            "\e[4m"
#define BLINK                "\e[5m"
#define REVERSE              "\e[7m"
#define HIDE                 "\e[8m"
#define CLEAR                "\e[2J"
#define CLRLINE              "\r\e[K" //or "\e[1K\r"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/* safe readable version of errno */
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define LOG_EMERG(M, ...)   do { fprintf(stderr, RED    "[EMERG]   " "%s (%s:%d) " NONE M YELLOW " errno: %s\n" NONE, __func__, __FILENAME__, __LINE__, ##__VA_ARGS__, clean_errno()); } while(0)
#define LOG_ALERT(M, ...)   do { fprintf(stderr, PURPLE "[ALERT]   " "%s (%s:%d) " NONE M YELLOW " errno: %s\n" NONE, __func__, __FILENAME__, __LINE__, ##__VA_ARGS__, clean_errno()); } while(0)
#define LOG_CRIT(M, ...)    do { fprintf(stderr, YELLOW "[CRIT]    " "%s (%s:%d) " NONE M YELLOW " errno: %s\n" NONE, __func__, __FILENAME__, __LINE__, ##__VA_ARGS__, clean_errno()); } while(0)
#define LOG_ERROR(M, ...)     do { fprintf(stderr, BROWN  "[ERR]     " "%s (%s:%d) " NONE M YELLOW " errno: %s\n" NONE, __func__, __FILENAME__, __LINE__, ##__VA_ARGS__, clean_errno()); } while(0)
#define LOG_WARNING(M, ...) do { fprintf(stderr, BLUE   "[WARNING] " "%s (%s:%d) " NONE M YELLOW " errno: %s\n" NONE, __func__, __FILENAME__, __LINE__, ##__VA_ARGS__, clean_errno()); } while(0)
#define LOG_NOTICE(M, ...)  do { fprintf(stderr, CYAN   "[NOTICE]  " "%s (%s:%d) " NONE M YELLOW " errno: %s\n" NONE, __func__, __FILENAME__, __LINE__, ##__VA_ARGS__, clean_errno()); } while(0)
#define LOG_INFO(M, ...)    do { fprintf(stderr, GREEN  "[INFO]    " "%s (%s:%d) " NONE M "\n", __func__, __FILENAME__, __LINE__, ##__VA_ARGS__); } while(0)
#define LOG_DEBUG(M, ...)   do { fprintf(stderr, GRAY   "[DEBUG]   " "%s (%s:%d) " NONE M "\n", __func__, __FILENAME__, __LINE__, ##__VA_ARGS__); } while(0)

/* LOG_LEVEL controls */
#if LOG_LEVEL < DEBUG
#undef LOG_DEBUG 
#define LOG_DEBUG(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < INFO
#undef LOG_INFO
#define LOG_INFO(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < NOTICE
#undef LOG_NOTICE
#define LOG_NOTICE(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < WARNING
#undef LOG_WARNING
#define LOG_WARNING(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < ERR
#undef LOG_ERROR
#define LOG_ERROR(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < CRIT
#undef LOG_CRIT
#define LOG_CRIT(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < ALERT
#undef LOG_ALERT
#define LOG_ALERT(M, ...) do{}while(0)
#endif

#if LOG_LEVEL < EMERG
#undef LOG_EMERG
#define LOG_EMERG(M, ...) do{}while(0)
#endif

/* LOG_COLOR controls */
#if LOG_COLOR < 1

#undef NONE
#define NONE

#undef RED
#define RED

#undef PURPLE
#define PURPLE

#undef YELLOW
#define YELLOW

#undef BROWN
#define BROWN

#undef GREEN
#define GREEN

#undef CYAN
#define CYAN

#undef BLUE
#define BLUE

#undef GRAY
#define GRAY

#endif

#endif // LWLOG_H_
