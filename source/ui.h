#pragma once
#include "common.h"

#define UI_STATE_STACK_DEPTH 4

typedef enum
{
	UI_STATE_NULL = 0,
	UI_STATE_MENU,
	UI_STATE_ERROR,

	UI_STATE_REBOOT,
	UI_STATE_TITLESELECT,
	UI_STATE_NETLOADER,

	UI_STATE_BACKGROUND,
	UI_STATE_MAX,
} UIState;

typedef struct
{
	void (* update)(void);
	void (* drawTop)(float iod);
	void (* drawBot)(void);
} uiStateInfo_s;

extern touchPosition g_touchPos;
extern circlePosition g_cstickPos;

extern const uiStateInfo_s g_uiStateTable[UI_STATE_MAX];
#define g_uiStateBg (&g_uiStateTable[UI_STATE_BACKGROUND])

void uiInit(void);
void uiEnterBusy(void);
void uiEnterState(UIState newState);
void uiExitState(void);
void uiExitLoop(void);
const uiStateInfo_s* uiGetStateInfo(void);
bool uiIsBusy(void);

bool uiUpdate(void);
