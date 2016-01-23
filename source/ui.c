#include "common.h"

// Global variables
touchPosition g_touchPos;
circlePosition g_cstickPos;

// Static variables
static UIState s_stateStack[UI_STATE_STACK_DEPTH];
static int s_stateStackPos = 1;
static bool s_stateBusy;
static bool s_shouldExit;

void uiInit(void)
{
	// Nothing here?
}

void uiEnterBusy(void)
{
	s_stateBusy = true;
}

void uiEnterState(UIState newState)
{
	s_stateBusy = false;
	if (s_stateStack[s_stateStackPos-1] == newState) return;
	if (s_stateStackPos>=UI_STATE_STACK_DEPTH) return;
	s_stateStack[s_stateStackPos++] = newState;
}

void uiExitState(void)
{
	if (s_stateStackPos<=1) return;
	s_stateStackPos--;
}

void uiExitLoop(void)
{
	s_shouldExit = true;
}

const uiStateInfo_s* uiGetStateInfo(void)
{
	UIState cur = s_stateStack[s_stateStackPos-1];
	return &g_uiStateTable[cur];
}

bool uiIsBusy(void)
{
	return s_stateBusy;
}

static int iabs(int x)
{
	return x < 0 ? (-x) : x;
}

bool uiUpdate(void)
{
	// Read input state
	hidScanInput();
	hidTouchRead(&g_touchPos);
	hidCstickRead(&g_cstickPos);
	g_cstickPos.dx = iabs(g_cstickPos.dx)<5 ? 0 : g_cstickPos.dx;
	g_cstickPos.dy = iabs(g_cstickPos.dy)<5 ? 0 : g_cstickPos.dy;

	const uiStateInfo_s* ui;

	// Update background
	ui = g_uiStateBg;
	if (ui->update) ui->update();

	if (!s_stateBusy)
	{
		// Update current state
		ui = uiGetStateInfo();
		if (ui->update) ui->update();
	}

	return !s_shouldExit;
}
