#pragma once
#include "../common.h"

void errorScreen(const char* title, const char* fmt, ...) __attribute__((format (printf, 2, 3)));
void errorUpdate(void);
void errorDrawBot(void);
