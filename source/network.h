#pragma once
#include "common.h"

#define NETWORK_PORT 17491

bool networkInit(void);
void networkDeactivate(void);
void networkError(bool netloader, const char* func, int err);
