#pragma once
#include "../common.h"

#define BUBBLE_COUNT 15

typedef struct
{
	s32 x, y;
	float z;
	u8 fade;
} bubble_t;

void backgroundInit(void);
void backgroundUpdate(void);
void backgroundDrawTop(float iod);
void backgroundDrawBot(void);
