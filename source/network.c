#include "network.h"

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
