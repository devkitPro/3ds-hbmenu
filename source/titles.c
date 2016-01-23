#include "titles.h"

typedef struct
{
	int totalNum;
	int filteredNum;
	u64* ids;
} titleList_s;

static titleList_s s_titles[3];

static void titleListClear(titleList_s* tl)
{
	if (tl->ids) free(tl->ids);
	memset(tl, 0, sizeof(*tl));
}

void titlesClear(void)
{
	int i;
	for (i = 0; i < 3; i ++)
		titleListClear(&s_titles[i]);
}

bool titlesExists(u64 tid, u8 mediatype)
{
	titleList_s* tl = &s_titles[mediatype];
	int i;
	for (i = 0; i < tl->filteredNum; i ++)
		if (tl->ids[i] == tid)
			return true;
	return false;
}

static void titlesUpdate(void* pnewState)
{
	int i, j;
	UIState newState = (UIState)pnewState;
	for (i = 0; i < 3; i ++)
	{
		titleList_s* tl = &s_titles[i];
		if (tl->totalNum == 0) goto _fail;
		u64* new_list = (u64*)realloc(tl->ids, tl->totalNum*sizeof(u64));
		if (!new_list) goto _fail;
		tl->ids = new_list;
		Result ret = AM_GetTitleIdList(i, tl->totalNum, tl->ids);
		if (R_FAILED(ret)) goto _fail;
		tl->filteredNum = 0;
		for (j = 0; j < tl->totalNum; j ++)
		{
			u64 tid = tl->ids[j];
			tl->ids[tl->filteredNum] = tid;
			u32 tid_high = tid >> 32;
			if (tid_high == 0x00040010 || tid_high == 0x00040000 || tid_high == 0x00040002)
				tl->filteredNum ++;
		}
		continue;

	_fail:
		titleListClear(tl);
	}

	if (newState > UI_STATE_NULL)
		uiEnterState(newState);
}

bool titlesCheckUpdate(bool async, UIState newState)
{
	int i;
	bool needUpdate = false;
	for (i = 0; i < 3; i ++)
	{
		int num;
		Result res = AM_GetTitleCount(i, (u32*)&num);
		if (R_SUCCEEDED(res) && s_titles[i].totalNum != num)
		{
			needUpdate = true;
			s_titles[i].totalNum = num;
		}
	}

	if (!needUpdate) return false;

	if (async)
		workerSchedule(titlesUpdate, (void*)newState);
	else
		titlesUpdate((void*)newState);
	return true;
}

Result titlesLoadSmdh(smdh_s* smdh, u8 mediatype, u64 tid)
{
	static const u32 filePath[] = {0, 0, 2, 0x6E6F6369, 0};
	Result res;
	u32 archivePath[] = {tid & 0xFFFFFFFF, tid >> 32, mediatype, 0};

	FS_Archive arch = { ARCHIVE_SAVEDATA_AND_CONTENT, { PATH_BINARY, sizeof(archivePath), archivePath }, 0 };
	FS_Path apath = { PATH_BINARY, sizeof(filePath), filePath };
	Handle file = 0;
	res = FSUSER_OpenFileDirectly(&file, arch, apath, FS_OPEN_READ, 0);
	if (R_FAILED(res)) return res;

	u32 bytesRead;
	res = FSFILE_Read(file, &bytesRead, 0, smdh, sizeof(*smdh));
	FSFILE_Close(file);

	return R_SUCCEEDED(res);
}
