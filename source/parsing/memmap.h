#pragma once
#include <3ds.h>

typedef struct
{
	u32 num;
	u32 text_end;
	u32 data_address;
	u32 data_size;
	u32 processLinearOffset;
	u32 processHookAddress;
	u32 processAppCodeAddress;
	u32 processHookTidLow, processHookTidHigh;
	u32 mediatype;
	bool capabilities[0x10]; // {socuAccess, csndAccess, qtmAccess, nfcAccess, httpcAccess, reserved...}
} memmap_header_t;

typedef struct
{
	u32 src, dst, size;
} memmap_entry_t;

typedef struct
{
	memmap_header_t header;
	memmap_entry_t map[];
} memmap_t;

#define memmapSize(m) (sizeof(memmap_header_t) + sizeof(memmap_entry_t)*(m)->header.num)
memmap_t* memmapLoad(const char* path);
