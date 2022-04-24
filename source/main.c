#include "common.h"
#include "ui/menu.h"
#include "ui/error.h"
#include "ui/reboot.h"
#include "ui/titleselect.h"
#include "ui/netloader.h"
#include "ui/netsender.h"
#include "ui/background.h"

const uiStateInfo_s g_uiStateTable[UI_STATE_MAX] =
{
	[UI_STATE_MENU]       = { .update = menuUpdate,       .drawTop = menuDrawTop,       .drawBot = menuDrawBot,       },
	[UI_STATE_ERROR]      = { .update = errorUpdate,                                    .drawBot = errorDrawBot,      },
	[UI_STATE_REBOOT]     = { .update = rebootUpdate,                                   .drawBot = rebootDrawBot,     },
	[UI_STATE_TITLESELECT]= { .update = titleSelectUpdate,                              .drawBot = titleSelectDrawBot,},
	[UI_STATE_NETLOADER]  = { .update = netloaderUpdate,                                .drawBot = netloaderDrawBot,  },
	[UI_STATE_NETSENDER]  = { .update = netsenderUpdate,                                .drawBot = netsenderDrawBot,  },
	[UI_STATE_BACKGROUND] = { .update = backgroundUpdate, .drawTop = backgroundDrawTop, .drawBot = backgroundDrawBot, },
};

static void startup(void* unused)
{
	#if __DEBUG__
		logFileInit();
	#endif

	menuLoadFileAssoc();
	menuScan("sdmc:/3ds");
	uiEnterState(UI_STATE_MENU);
}

const char* __romfs_path = "sdmc:/boot.3dsx";

int main()
{
	Result rc;

	osSetSpeedupEnable(true);
	rc = romfsInit();
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	menuStartupPath();
	hidSetRepeatParameters(20, 10);
	ptmuInit();
	uiInit();
	drawingInit();
	textInit();
	workerInit();
	launchInit();

	backgroundInit();

	workerSchedule(startup, NULL);

	// Main loop
	while (aptMainLoop())
	{
		if (!uiUpdate()) break;
		drawingFrame();
	}

	netloaderExit();
	netsenderExit();
	launchExit();
	workerExit();
	textExit();
	drawingExit();
	romfsExit();
	ptmuExit();
	return 0;
}
