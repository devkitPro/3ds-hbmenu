#pragma once
#include <3ds.h>

#define _3DSX_MAGIC 0x58534433 // '3DSX'
#define _3DSX_HEADER_SIZE offsetof(_3DSX_Header, smdhOffset)

// File header
typedef struct
{
	u32 magic;
	u16 headerSize, relocHdrSize;
	u32 formatVer;
	u32 flags;

	// Sizes of the code, rodata and data segments +
	// size of the BSS section (uninitialized latter half of the data segment)
	u32 codeSegSize, rodataSegSize, dataSegSize, bssSize;

	// Extended header below:

	// offset and size of smdh
	u32 smdhOffset, smdhSize;
	// offset to filesystem
	u32 fsOffset;
} _3DSX_Header;
