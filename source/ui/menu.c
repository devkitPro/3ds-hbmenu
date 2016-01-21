#include "menu.h"

static void changeDirTask(void* arg)
{
	menuScan((const char*)arg);
	uiEnterState(UI_STATE_MENU);
}

void menuUpdate(void)
{
	menu_s* menu = menuGetCurrent();
	u32 down = hidKeysDown();
	u32 held = hidKeysHeld();
	if (down & KEY_A)
	{
		//workerSchedule(menuTask, NULL);
		workerSchedule(changeDirTask, "sdmc:/3ds");
	}
	else if (down & KEY_B)
	{
		workerSchedule(changeDirTask, "..");
	}
	else if (down & KEY_START)
	{
		uiEnterState(UI_STATE_REBOOT);
	}
	else if (down & KEY_Y)
	{
		uiEnterState(UI_STATE_NETLOADER);
	}
	else
	{
		if (held & KEY_UP)
			menu->scrollLocation += 4.0f;
		else if (held & KEY_DOWN)
			menu->scrollLocation -= 4.0f;
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

static float menuDrawEntry(menuEntry_s* me, float x, float y, bool selected)
{
	float bubbleWidth = g_imageData[imgId_appbubble].width;
	float bubbleHeight = g_imageData[imgId_appbubble].height;

	float height = bubbleHeight + 4.0f;
	if (selected)
	{
		height += 4.0f;
		x -= 4.0f;
	}

	if ((y+height) <= 0.0f || y >= 240.f)
		return height;

	if (selected)
		drawingDrawImage(imgId_appbubble, 0x80808080, x, y+4);
	drawingDrawImage(imgId_appbubble, 0xFFFFFFFF, x, y);

	if (me->icon)
	{
		drawingWithTex(me->icon, 0xFFFFFFFF);
		drawingDrawQuad(x+8, y+8, 48, 48);
	} else if (me->type == ENTRY_TYPE_FOLDER)
		drawingDrawImage(imgId_folderIcon, 0xFFFFFFFF, x+8, y+8);
	else
		drawingDrawImage(imgId_defaultIcon, 0xFFFFFFFF, x+8, y+8);

	textSetColor(0xFF545454);
	textDrawInBox(me->name, -1, 0.5f, 0.5f, y+20, x+66, x+bubbleWidth-8);
	textDrawInBox(me->description, -1, 0.4f, 0.4f, y+35, x+66+4, x+bubbleWidth-8);
	textDrawInBox(me->author, 1, 0.4f, 0.4f, y+bubbleHeight-6, x+66, x+bubbleWidth-8);

	return height;
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
		textDrawInBox("No applications found", 0, 0.75f, 0.75f, 60.0f+25.0f, 8.0f, 320-8.0f);
		textDraw(8.0f, 60.0f+25.0f+8.0f, 0.5f, 0.5f, false,
			"No applications could be found on the SD card.\n"
			"Make sure a folder named /3ds exists in the\n"
			"root of the SD card and it contains applications."
		);
		return;
	} else
	{
		float y = 0.0;
		for (me = menu->firstEntry, i = 0; me; me = me->next, i ++)
			y += menuDrawEntry(me, 9.0f, 4+y+menu->scrollLocation, i==menu->curEntry);

		drawingDrawImage(imgId_scrollbarTrack, 0xFFFFFFFF, 308.0f, 5.0f);
	}

	drawingDrawImage(imgId_settings, 0xFFFFFFFF, 320.0f-g_imageData[imgId_settings].width, 240.0f-g_imageData[imgId_settings].height);
}
