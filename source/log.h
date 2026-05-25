#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

void logInit(void);
int logPrintf(const char* format, ...);
void logClose(void);

#ifndef LOG_INTERNAL
// Macro para redirigir automaticamente todos los printf del proyecto a logPrintf
#define printf(...) logPrintf(__VA_ARGS__)
#endif

#endif // LOG_H
