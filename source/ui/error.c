#include "error.h"
#include <stdarg.h>

static const char* errorTitle;
static char errorText[256];

void errorInit(const char* title, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vsnprintf(errorText, sizeof(errorText), fmt, va);
	va_end(va);
	errorTitle = title;
	uiEnterState(UI_STATE_ERROR);
}

void errorUpdate(void)
{
	if (hidKeysDown() & KEY_B)
		uiExitState();
}

void errorDrawBot(void)
{
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x80FFFFFF);
	drawingDrawQuad(0.0f, 60.0f, 320.0f, 120.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	textSetColor(0xFF545454);
	textDrawInBox(errorTitle, 0, 0.75f, 0.75f, 60.0f+25.0f, 8.0f, 320-8.0f);
	textDraw(8.0f, 60.0f+25.0f+8.0f, 0.5f, 0.5f, false, errorText);
}
