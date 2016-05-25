#include "../common.h"
#include "../parsing/memmap.h"

typedef struct
{
	s32 processId;
	bool capabilities[0x10];
} processEntry_s;

typedef void (*callBootloader_2x_fn)(Handle file, u32* argbuf, u32 arglength);
typedef void (*callBootloaderNewProcess_2x_fn)(s32 processId, u32* argbuf, u32 arglength);
typedef void (*callBootloaderRunTitle_2x_fn)(u8 mediatype, u32* argbuf, u32 argbuflength, u32 tid_low, u32 tid_high);
typedef void (*callBootloaderRunTitleCustom_2x_fn)(u8 mediatype, u32* argbuf, u32 argbuflength, u32 tid_low, u32 tid_high, memmap_t* mmap);
typedef void (*getBestProcess_2x_fn)(u32 sectionSizes[3], bool* requirements, int num_requirements, processEntry_s* out, int out_size, int* out_len);

#define callBootloader_2x ((callBootloader_2x_fn)0x00100000)
#define callBootloaderNewProcess_2x ((callBootloaderNewProcess_2x_fn)0x00100008)
#define callBootloaderRunTitle_2x ((callBootloaderRunTitle_2x_fn)0x00100010)
#define callBootloaderRunTitleCustom_2x ((callBootloaderRunTitleCustom_2x_fn)0x00100014)
#define getBestProcess_2x ((getBestProcess_2x_fn)0x0010000C)

static s32 targetProcess = -1;
static u64 targetTid;
static u8 targetMediatype;
static Handle fileHandle;
static u32 argBuf[ENTRY_ARGBUFSIZE/sizeof(u32)];
static u32 argBufLen;
static u32 memMapBuf[0x40];
static bool useMemMap;

static bool init(void)
{
	return R_SUCCEEDED(amInit());
}

static void deinit(void)
{
	amExit();
}

static void bootloaderJump(void)
{
	if (targetProcess == -1)
		callBootloader_2x(fileHandle, argBuf, argBufLen);
	else if (targetProcess == -2)
	{
		if (useMemMap)
			callBootloaderRunTitleCustom_2x(targetMediatype, argBuf, argBufLen, (u32)targetTid, (u32)(targetTid>>32), (memmap_t*)memMapBuf);
		else
			callBootloaderRunTitle_2x(targetMediatype, argBuf, argBufLen, (u32)targetTid, (u32)(targetTid>>32));
	}
	else
		callBootloaderNewProcess_2x(targetProcess, argBuf, argBufLen);
}

static void launchFile(const char* path, argData_s* args, executableMetadata_s* em)
{
	if (em && em->scanned && targetProcess == -1)
	{
		// this is a really shitty implementation of what we should be doing
		// i'm really too lazy to do any better right now, but a good solution will come
		// (some day)
		processEntry_s out[4];
		int out_len = 0;
		getBestProcess_2x(em->sectionSizes, (bool*)em->servicesThatMatter, NUM_SERVICESTHATMATTER, out, 4, &out_len);

		// temp : check if we got all the services we want
		if (
			em->servicesThatMatter[0] <= out[0].capabilities[0]
			&& em->servicesThatMatter[1] <= out[0].capabilities[1]
			&& em->servicesThatMatter[2] <= out[0].capabilities[2]
			&& em->servicesThatMatter[3] <= out[0].capabilities[3]
			&& em->servicesThatMatter[4] <= out[0].capabilities[4])
			targetProcess = out[0].processId;
		else
		{
			// temp : if we didn't get everything we wanted, we search for the candidate that has as many highest-priority services as possible
			int i, j;
			int best_id = 0;
			int best_sum = 0;
			for (i=0; i<out_len; i++)
			{
				int sum = 0;
				for(j=0; j<NUM_SERVICESTHATMATTER; j++)
					sum += (em->servicesThatMatter[j] == 1) && out[i].capabilities[j];

				if(sum > best_sum)
				{
					best_id = i;
					best_sum = sum;
				}
			}
			targetProcess = out[best_id].processId;
		}
	} else if (targetProcess != -1)
		targetProcess = -2;

	if (targetProcess == -1)
	{
		fileHandle = launchOpenFile(path);
		if (fileHandle==0)
		{
			errorInit(textGetString(StrId_IOError), textGetString(StrId_CouldNotOpenFile), path);
			return;
		}
	}

	argBufLen = args->dst - (char*)args->buf;
	memcpy(argBuf, args->buf, argBufLen);
	__system_retAddr = bootloaderJump;
	uiExitLoop();
}

static void useTitle(u64 tid, u8 mediatype)
{
	targetProcess = -2;
	targetTid = tid;
	targetMediatype = mediatype;

	char buf[32];
	sprintf(buf, "/mmap/%08lX%08lX.xml", (u32)(tid>>32), (u32)tid);
	memmap_t* map = memmapLoad(buf);
	if (map)
	{
		u32 size = memmapSize(map);
		if (size <= sizeof(memMapBuf))
		{
			useMemMap = true;
			memcpy(memMapBuf, map, size);
		}
		free(map);
	}
}

const loaderFuncs_s loader_Ninjhax2 =
{
	.name = "hax 2.x",
	.init = init,
	.deinit = deinit,
	.launchFile = launchFile,
	.useTitle = useTitle,
};
