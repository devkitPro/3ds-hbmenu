#include "reboot.h"

static bool rebooting = false;

void rebootUpdate(void)
{
	if (rebooting) return;

	u32 down = hidKeysDown();
	if ((down & KEY_A) || (down & KEY_X))
	{
		rebooting = true;
		drawingSetFade(-1.0/60);

		if (!(down & KEY_X) && launchHomeMenuEnabled())
			launchHomeMenu();
		else
		{
			aptOpenSession();
			APT_HardwareResetAsync();
			aptCloseSession();
		}
		return;
	}

	if (down & KEY_B)
	{
		uiExitState();
		return;
	}
}

void rebootDrawBot(void)
{
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x80FFFFFF);
	drawingDrawQuad(0.0f, 60.0f, 320.0f, 120.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	static const char* const msg1 =
		"Returning to home menu is not available.\n"
		"You're about to reboot your console.\n\n"
		"  \xEE\x80\x80 Proceed\n"
		"  \xEE\x80\x81 Cancel";

	static const char* const msg2 =
		"You're about to return to home menu.\n\n"
		"  \xEE\x80\x80 Proceed\n"
		"  \xEE\x80\x81 Cancel\n"
		"  \xEE\x80\x82 Reboot";

	//textSetColor(0xFF767676);
	textSetColor(0xFF545454);
	textDraw(8.0f, 60.0f+8.0f, 0.6f, 0.6f, false, launchHomeMenuEnabled() ? msg2 : msg1);
}
