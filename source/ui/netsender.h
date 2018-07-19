#pragma once
#include "../common.h"

#define NETSENDER_PORT 17491

void netsenderTask(void* arg);
void netsenderUpdate(void);
void netsenderDrawBot(void);
void netsenderExit(void);
