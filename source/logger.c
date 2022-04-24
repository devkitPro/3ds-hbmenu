#include "logger.h"
#include <stdio.h>

static FILE* logFile = NULL;
static const char* LOG_FORMAT = "%s:%zu:\n%s\n\n";

void logFileInit(void) {
    logFile = freopen("hbmenu.log", "w", stderr);
}

void logFileOutput(const char* func, size_t line, const char* format, ...) {
    if (!logFile)
        return;

    va_list args;
    va_start(args, format);
    char buffer[255];

    vsnprintf(buffer, sizeof(buffer), format, args);
    fprintf(logFile, LOG_FORMAT, func, line, buffer);

    fflush(logFile);
}
