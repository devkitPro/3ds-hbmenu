#pragma once
#include "../common.h"

#define BUBBLE_COUNT 64

typedef struct
{
	s32 x, y;
	float z;
	float angle;
	float angv;
	u8 fade;
} bubble_t;

void backgroundInit(void);
void backgroundUpdate(void);
void backgroundDrawTop(float iod);
void backgroundDrawBot(void);
