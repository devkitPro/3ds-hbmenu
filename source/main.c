#include "common.h"
#include "ui/menu.h"
#include "ui/reboot.h"
#include "ui/background.h"

const uiStateInfo_s g_uiStateTable[UI_STATE_MAX] =
{
	[UI_STATE_MENU]       = { .update = menuUpdate,       .drawTop = menuDrawTop,       .drawBot = menuDrawBot,       },
	[UI_STATE_REBOOT]     = { .update = rebootUpdate,                                   .drawBot = rebootDrawBot,     },
	[UI_STATE_BACKGROUND] = { .update = backgroundUpdate, .drawTop = backgroundDrawTop, .drawBot = backgroundDrawBot, },
};

static void startup(void* unused)
{
	menuScan("sdmc:/3ds");
	uiEnterState(UI_STATE_MENU);
}

int main()
{
	Result rc;

	osSetSpeedupEnable(true);
	sdmcWriteSafe(false);
	rc = romfsInit();
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	acInit();
	ptmuInit();
	uiInit();
	drawingInit();
	textInit();
	workerInit();

	backgroundInit();

	workerSchedule(startup, NULL);

	// Main loop
	while (aptMainLoop())
	{
		if (!uiUpdate()) break;
		drawingFrame();
	}

	workerExit();
	textExit();
	drawingExit();
	romfsExit();
	ptmuExit();
	acExit();
	return 0;
}
