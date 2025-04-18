#ifndef __LOG_H__
#define __LOG_H__

#define LOG_PRINTF(_fmt_, ...)				printf(_fmt_"\r\n", ##__VA_ARGS__)
#define LOG_ERROR(_fmt_, ...)               printf("[error] "_fmt_" (%s, line %d)", ##__VA_ARGS__, __FUNCTION__, __LINE__)

#endif