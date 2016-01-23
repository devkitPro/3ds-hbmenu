#include "titleselect.h"
#include "menu.h"

#define UPDATE_FREQUENCY 250

static menuEntry_s* s_launchEntry;
static menuEntry_s s_iconEntry;
static int s_curTitle;
static bool s_iconReady;
static u64 s_curTitleID;
static u8 s_curMediatype;
static u64 s_lastUpdate;

static void loadTitleData(void* unused)
{
	titlesGetEntry(&s_curTitleID, &s_curMediatype, s_curTitle);
	if (R_FAILED(titlesLoadSmdh(&s_iconEntry.smdh, s_curMediatype, s_curTitleID)))
	{
		s_curTitle = -1;
		errorInit("Title selector", "Error reading title metadata.\n%08lX%08lX@%d",
			(u32)(s_curTitleID>>32), (u32)s_curTitleID, s_curMediatype);
		return;
	}

	s_iconReady = false;
	menuEntryParseSmdh(&s_iconEntry);
	s_iconReady = true;
	uiEnterState(UI_STATE_TITLESELECT);
}

void titleSelectInit(menuEntry_s* me)
{
	s_launchEntry = me;
	titlesCheckUpdate(false, UI_STATE_NULL);
	s_lastUpdate = osGetTime();
	if (!titlesCount())
	{
		errorInit("Title selector", "No titles could be detected.");
		return;
	}
	s_curTitle = -1;
	menuEntryInit(&s_iconEntry, ENTRY_TYPE_FILE);
	uiEnterState(UI_STATE_TITLESELECT);
}

static void launchEntry(void* unused)
{
	launchMenuEntry(s_launchEntry);
}

void titleSelectUpdate(void)
{
	u32 kDown = hidKeysDown();

	if (s_curTitle < 0)
	{
		s_curTitle = 0;
		workerSchedule(loadTitleData, NULL);
		return;
	}

	u64 curTime = osGetTime();
	if ((curTime-s_lastUpdate) > UPDATE_FREQUENCY)
	{
		s_lastUpdate = curTime;
		if (titlesCheckUpdate(true, UI_STATE_TITLESELECT))
		{
			s_curTitle = -1;
			return;
		}
	}

	if ((kDown & KEY_A) || (kDown & KEY_B))
	{
		// Free entry.
		menuEntryFree(&s_iconEntry);
		uiExitState();
		if (kDown & KEY_A)
		{
			s_launchEntry->titleSelected = true;
			s_launchEntry->titleId = s_curTitleID;
			s_launchEntry->titleMediatype = s_curMediatype;
			workerSchedule(launchEntry, NULL);
		}
		return;
	}

	int move = 0;
	if (kDown & KEY_RIGHT) move++;
	if (kDown & KEY_LEFT) move--;

	int tgt = s_curTitle+move;
	int cnt = titlesCount();
	while (tgt < 0) tgt += cnt;
	while (tgt >= cnt) tgt -= cnt;

	if (tgt != s_curTitle)
	{
		s_curTitle = tgt;
		workerSchedule(loadTitleData, NULL);
		return;
	}
}

void titleSelectDrawBot(void)
{
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x80FFFFFF);
	drawingDrawQuad(0.0f, 10.0f, 320.0f, 120.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	textSetColor(0xFF545454);
	textDrawInBox("Title selector", 0, 0.75f, 0.75f, 10.0f+25.0f, 8.0f, 320-8.0f);
	textDraw(8.0f, 10.0f+25.0f+8.0f, 0.5f, 0.5f, false,
		"Please select a target title.\n\n"
		"  \xEE\x80\x80 Select\n"
		"  \xEE\x80\x81 Cancel"
		);

	if (s_iconReady)
	{
		float x = (320.0f-g_imageData[imgId_appbubble].width)/2+4;
		menuDrawEntry(&s_iconEntry, x, 150.0f-4, true);
	}
}
