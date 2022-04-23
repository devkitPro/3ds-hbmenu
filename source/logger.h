#pragma once

#include <stdarg.h>
#include <stdio.h>

void logFileInit(void);
void logFileOutput(const char* func, size_t line, const char* format, ...);

#define LOG(format, ...) \
    logFileOutput(__PRETTY_FUNCTION__, __LINE__, format, ##__VA_ARGS__)
