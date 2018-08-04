#include "network.h"

#include <malloc.h>

static void* SOC_buffer;

bool networkInit(void)
{
	if (!SOC_buffer)
	{
		SOC_buffer = memalign(0x1000, 0x100000);
		if (!SOC_buffer)
			return false;
	}

	Result ret = socInit(SOC_buffer, 0x100000);
	if (R_FAILED(ret))
	{
		socExit();
		return false;
	}
	return true;
}

void networkDeactivate(void)
{
	socExit();
	if(SOC_buffer)
		free(SOC_buffer);
}

// xError in their own files call xDeactivate beforehand
void networkError(void (* update)(void), StrId titleStrId, const char* func, int err)
{
	if (uiGetStateInfo()->update == update)
		uiExitState();
	errorScreen(textGetString(titleStrId), textGetString(StrId_NetLoaderError), func, err);
}

void networkDrawBot(StrId titleStrId, const char* other, bool transferring, size_t filelen, size_t filetotal)
{
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x80FFFFFF);
	drawingDrawQuad(0.0f, 60.0f, 320.0f, 120.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	textSetColor(0xFF545454);
	textDrawInBox(textGetString(titleStrId), 0, 0.75f, 0.75f, 60.0f+25.0f, 8.0f, 320-8.0f);

	char buf[256];
	const char* text = other;
	if(text == NULL)
	{
		text = buf;
		u32 ip = gethostid();

		if (ip == 0)
			snprintf(buf, sizeof(buf), textGetString(StrId_NetLoaderOffline));
		else
			snprintf(buf, sizeof(buf), textGetString(StrId_NetLoaderTransferring), filetotal/1024, filelen/1024);
	}

	textDraw(8.0f, 60.0f+25.0f+8.0f, 0.5f, 0.5f, false, text);

	if (transferring)
	{
		float progress = (float)filetotal / filelen;
		float width = progress*320;

		drawingWithColor(0xC000E000);
		drawingDrawQuad(0.0f, 60.0f+120.0f-16.0f, width, 16.0f);
		drawingWithColor(0xC0C0C0C0);
		drawingDrawQuad(width, 60.0f+120.0f-16.0f, 320.0f-width, 16.0f);

		snprintf(buf, sizeof(buf), "%.02f%%", progress*100);
		textSetColor(0xFF000000);
		textDrawInBox(buf, 0, 0.5f, 0.5f, 60.0f+120.0f-3.0f, 0.0f, 320.0f);
	}
}
