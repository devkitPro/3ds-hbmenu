#pragma once
#include "common.h"

#define NETWORK_PORT 17491

bool networkInit(void);
void networkDeactivate(void);
void networkError(void (* update)(void), StrId titleStrId, const char* func, int err);
void networkDrawBot(StrId titleStrId, const char* other, bool transferring, size_t filelen, size_t filetotal);
