#pragma once
#include "../common.h"

#define NETLOADER_PORT 17491

void netloaderTask(void* arg);
void netloaderUpdate(void);
void netloaderDrawBot(void);
void netloaderExit(void);
