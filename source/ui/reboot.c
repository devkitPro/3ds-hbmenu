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

	if (down & KEY_Y)
	{
		// Check a gamecard is inserted.
		if (titlesLoadSmdh(NULL, 2, 0))
		{
			// Check we have access to ns:s.
			Result rc = nsInit();
			if (R_SUCCEEDED(rc))
			{
				// Reboot the console.
				rebooting = true;
				drawingSetFade(-1.0/60);
				NS_RebootToTitle(2, 0);
				nsExit();
				return;
			}
		}
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

	//textSetColor(0xFF767676);
	textSetColor(0xFF545454);
	textDraw(8.0f, 60.0f+8.0f, 0.6f, 0.6f, false, textGetString(launchHomeMenuEnabled() ? StrId_ReturnToHome : StrId_Reboot));
}
