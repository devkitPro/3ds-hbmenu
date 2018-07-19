#include "menu.h"
#include "netloader.h"
#include "netsender.h"

static bool showingHomeIcon;
static float homeIconStatus;

static void changeDirTask(void* arg)
{
	menuScan((const char*)arg);
	uiEnterState(UI_STATE_MENU);
}

static void launchMenuEntryTask(void* arg)
{
	menuEntry_s* me = (menuEntry_s*)arg;
	if (me->type == ENTRY_TYPE_FOLDER)
		changeDirTask(me->path);
	else
		launchMenuEntry(me);
}

static float menuGetScrollHeight(menu_s* menu)
{
	float ret = 4.0f + menu->nEntries*(4.0f+g_imageData[images_appbubble_idx].height) - 240.0f;
	if (ret < 0.0f) ret = 0.0f;
	return ret;
}

static int iabs(int x)
{
	return x<0 ? (-x) : x;
}

static float menuGetEntryPos(menu_s* menu, int order)
{
	float ret = order*(4.0f+g_imageData[images_appbubble_idx].height);
	if (order > menu->curEntry)
		ret += 4.0f;
	return ret;
}

static void menuUpdateAnimation(menu_s* menu, float maxScroll, float val)
{
	if (menu->perturbed)
	{
		menu->scrollTarget = menuGetEntryPos(menu, menu->curEntry) - menu->scrollLocation;
		float borderBot = 240.0f - g_imageData[images_appbubble_idx].height - 4;
		if (menu->scrollTarget > borderBot || (maxScroll > 0.0f && menu->curEntry==(menu->nEntries-1)))
			menu->scrollVelocity += (menu->scrollTarget - borderBot) / SCROLLING_SPEED;
		if (menu->scrollTarget < 0.0f || (maxScroll > 0.0f && menu->curEntry==0))
			menu->scrollVelocity += menu->scrollTarget / SCROLLING_SPEED;
	} else if (maxScroll > 0.0f)
	{
		if (menu->scrollLocation >= maxScroll)
		{
			menu->scrollVelocity += (maxScroll - menu->scrollLocation) / SCROLLING_SPEED;
			if (val > 0.0f) menu->scrollVelocity -= val;
		} else if (menu->scrollLocation < 0.0f)
		{
			menu->scrollVelocity -= menu->scrollLocation / SCROLLING_SPEED;
			if (val < 0.0f) menu->scrollVelocity -= val;
		} else
			menu->scrollVelocity -= val;
	}

	menu->scrollLocation += menu->scrollVelocity;
	menu->scrollVelocity *= 0.75f;
	if (fabs(menu->scrollVelocity) < (1.0f/1024))
	{
		menu->scrollVelocity = 0.0f;
		menu->perturbed = false;
	}
}

void menuUpdate(void)
{
	menu_s* menu = menuGetCurrent();
	u32 down = hidKeysDown();
	u32 held = hidKeysHeld();
	u32 up   = hidKeysUp();
	bool pressedSettings = (down & KEY_TOUCH) && g_touchPos.px >= (320-g_imageData[images_settings_idx].width) && g_touchPos.py >= (240-g_imageData[images_settings_idx].height);
	if (down & KEY_A)
	{
		if (menu->nEntries > 0)
		{
			int i;
			menuEntry_s* me;
			for (i = 0, me = menu->firstEntry; i != menu->curEntry; i ++, me = me->next);
			workerSchedule(launchMenuEntryTask, me);
		}
	}
	else if (down & KEY_B)
	{
		workerSchedule(changeDirTask, "..");
	}
	else if ((down & KEY_START) || pressedSettings)
	{
		if (loaderHasFlag(LOADER_SHOW_REBOOT))
			uiEnterState(UI_STATE_REBOOT);
		else
			showingHomeIcon = true;
	}
	else if (down & KEY_Y)
	{
		workerSchedule(netloaderTask, NULL);
	}
	else if (down & KEY_X)
	{
		int i;
		menuEntry_s* me;
		for (i = 0, me = menu->firstEntry; i != menu->curEntry; i ++, me = me->next);
		workerSchedule(netsenderTask, me);
	}
	else if (menu->nEntries > 0)
	{
		u32 i;
		int move = 0;

		float val = g_cstickPos.dy * 1.0f/64;

		if (down & KEY_UP) move--;
		if (down & KEY_DOWN) move++;
		if (down & KEY_LEFT) move-=4;
		if (down & KEY_RIGHT) move+=4;

		int oldEntry = menu->curEntry;

		if (down & KEY_TOUCH)
		{
			menu->touchTimer = 0;
			menu->firstTouch = g_touchPos;
		} else if (held & KEY_TOUCH)
		{
			val += (g_touchPos.py - menu->previousTouch.py) * 16.0f/64;
			menu->touchTimer += drawingGetFrames();
		} else if ((up & KEY_TOUCH) && menu->touchTimer<30
			&& (iabs(menu->firstTouch.px-menu->previousTouch.px)+iabs(menu->firstTouch.py-menu->previousTouch.py))<12)
		{
			menuEntry_s *me, *me_sel = NULL;
			float location = menu->scrollLocation + menu->previousTouch.py-4;

			for (i = 0, me = menu->firstEntry; me; i++, me = me->next)
			{
				if (menuGetEntryPos(menu, i) > location)
					break;
				me_sel = me;
			}

			if (!me_sel)
			{
				me_sel = menu->firstEntry;
				i = 1;
			}

			if (menu->curEntry == (i-1))
			{
				workerSchedule(launchMenuEntryTask, me_sel);
				return;
			}

			menu->curEntry = i-1;
		}

		int newEntry = menu->curEntry + move;
		if (newEntry < 0) newEntry = 0;
		if (newEntry >= menu->nEntries) newEntry = menu->nEntries-1;
		menu->curEntry = newEntry;

		if (oldEntry != newEntry)
			menu->perturbed = true;

		menu->previousTouch = g_touchPos;
		for (i = drawingGetFrames(); i; i --)
			menuUpdateAnimation(menu, menuGetScrollHeight(menu), val);
	}
}

void menuDrawTop(float iod)
{
	menu_s* menu = menuGetCurrent();

	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x60FFFFFF);
	drawingDrawQuad(0.0f, 240-24.0f, 400.0f, 24.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	textSetColor(0xFF545454);
	textDrawInBox(menu->dirname, 0, 0.5f, 0.5f, 240.0f-8, 8.0f, 400-8.0f);
}

float menuDrawEntry(menuEntry_s* me, float x, float y, bool selected)
{
	float bubbleWidth = g_imageData[images_appbubble_idx].width;
	float bubbleHeight = g_imageData[images_appbubble_idx].height;

	float height = bubbleHeight + 4.0f;
	if (selected)
	{
		height += 4.0f;
		x -= 4.0f;
	}

	if ((y+height) <= 0.0f || y >= 240.f)
		return height;

	if (selected)
		drawingDrawImage(images_appbubble_idx, 0x80808080, x, y+4);
	drawingDrawImage(images_appbubble_idx, 0xFFFFFFFF, x, y);

	if (me->icon)
	{
		drawingWithTex(me->icon, 0xFFFFFFFF);
		drawingDrawQuad(x+8, y+8, 48, 48);
	} else if (me->type == ENTRY_TYPE_FOLDER)
		drawingDrawImage(images_folderIcon_idx, 0xFFFFFFFF, x+8, y+8);
	else
		drawingDrawImage(images_defaultIcon_idx, 0xFFFFFFFF, x+8, y+8);

	textSetColor(0xFF545454);
	textDrawInBox(me->name, -1, 0.5f, 0.5f, y+20, x+66, x+bubbleWidth-8);
	textDrawInBox(me->description, -1, 0.4f, 0.4f, y+35, x+66+4, x+bubbleWidth-8);
	textDrawInBox(me->author, 1, 0.4f, 0.4f, y+bubbleHeight-6, x+66, x+bubbleWidth-8);

	return height;
}

static float calcScrollbarKnobPos(menu_s* menu, float totalHeight)
{
	totalHeight -= 240.0f;
	if (totalHeight < 0.0f) return 5.0f;
	float trackHeight = g_imageData[images_scrollbarTrack_idx].height-g_imageData[images_scrollbarKnob_idx].height;
	float curPos = menu->scrollLocation;
	if (curPos < 0.0f) curPos = 0.0f;
	else if (curPos >= totalHeight) curPos = totalHeight;
	return 5.0f + curPos*trackHeight/totalHeight;
}

void menuDrawBot(void)
{
	int i;
	menuEntry_s* me;
	menu_s* menu = menuGetCurrent();

	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	if (menu->nEntries==0)
	{
		drawingWithColor(0x80FFFFFF);
		drawingDrawQuad(0.0f, 60.0f, 320.0f, 120.0f);
		drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

		textSetColor(0xFF545454);
		textDrawInBox(textGetString(StrId_NoAppsFound_Title), 0, 0.75f, 0.75f, 60.0f+25.0f, 8.0f, 320-8.0f);
		textDraw(8.0f, 60.0f+25.0f+8.0f, 0.5f, 0.5f, false, textGetString(StrId_NoAppsFound_Msg));
	} else
	{
		// Draw menu entries
		float y = 0.0;
		float loc = floorf(menu->scrollLocation);
		for (me = menu->firstEntry, i = 0; me; me = me->next, i ++)
			y += menuDrawEntry(me, 9.0f, 4+y-loc, i==menu->curEntry);

		// Draw scrollbar
		drawingDrawImage(images_scrollbarTrack_idx, 0xFFFFFFFF, 308.0f, 5.0f);
		drawingDrawImage(images_scrollbarKnob_idx,  0xFFFFFFFF, 308.0f, calcScrollbarKnobPos(menu, y));
	}

	drawingDrawImage(images_settings_idx, 0xFFFFFFFF, 320.0f-g_imageData[images_settings_idx].width, 240.0f-g_imageData[images_settings_idx].height);

	if (showingHomeIcon)
	{
		const float maxOpac = 0.75f;
		homeIconStatus += 1.0/32;
		float opac = 0.0;
		if (homeIconStatus < 1.0f)
			opac = maxOpac*sqrtf(homeIconStatus);
		else if (homeIconStatus < 2.0f)
			opac = maxOpac;
		else if (homeIconStatus < 3.0f)
			opac = maxOpac*sqrtf(3.0f-homeIconStatus);
		if (opac > 0.0f)
		{
			textSetColor((u32)(opac*0x100) << 24);
			textDrawInBox("\xEE\x81\xB3\n▼", 0, 1.0f, 1.0f, 200.0f, 0.0f, 320.0f);
		} else
		{
			showingHomeIcon = false;
			homeIconStatus = 0.0f;
		}
	}
}
