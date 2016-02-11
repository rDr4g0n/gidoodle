#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "debug.h"

void debug(const char *fmt, ...) {
    if(DEBUG){
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}
