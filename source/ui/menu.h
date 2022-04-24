#pragma once
#include "../common.h"

#define SCROLLING_SPEED 16

void menuUpdate(void);
void menuDrawTop(float iod);
void menuDrawBot(void);
void menuLoadFileAssoc(void);

float menuDrawEntry(menuEntry_s* me, float x, float y, bool selected);
