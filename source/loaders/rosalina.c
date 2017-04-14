#include "../common.h"

static Handle hbldrHandle;

static bool init(void)
{
	Result res = svcConnectToPort(&hbldrHandle, "hb:ldr");
	if (R_FAILED(res))
		return false;

	res = amInit();
	if (R_FAILED(res))
	{
		svcCloseHandle(hbldrHandle);
		return false;
	}

	return true;
}

static Result HBLDR_SetTarget(const char* path)
{
	u32 pathLen = strlen(path) + 1;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(2, 0, 2); //0x20002
	cmdbuf[1] = IPC_Desc_StaticBuffer(pathLen, 0);
	cmdbuf[2] = (u32)path;

	Result rc = svcSendSyncRequest(hbldrHandle);
	if (R_SUCCEEDED(rc)) rc = cmdbuf[1];
	return rc;
}

static Result HBLDR_SetArgv(const void* buffer, u32 size)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(3, 0, 2); //0x30002
	cmdbuf[1] = IPC_Desc_StaticBuffer(size, 1);
	cmdbuf[2] = (u32)buffer;

	Result rc = svcSendSyncRequest(hbldrHandle);
	if (R_SUCCEEDED(rc)) rc = cmdbuf[1];
	return rc;
}

static void deinit(void)
{
	amExit();
	svcCloseHandle(hbldrHandle);
}

static void launchFile(const char* path, argData_s* args, executableMetadata_s* em)
{
	if (strncmp(path, "sdmc:/",6) == 0)
		path += 5;
	HBLDR_SetTarget(path);
	HBLDR_SetArgv(args->buf, sizeof(args->buf));
	uiExitLoop();
}

static void useTitle(u64 tid, u8 mediatype)
{
	aptSetChainloader(tid, mediatype);
}

const loaderFuncs_s loader_Rosalina =
{
	.name = "Rosalina",
	.init = init,
	.deinit = deinit,
	.launchFile = launchFile,
	.useTitle = useTitle,
};
