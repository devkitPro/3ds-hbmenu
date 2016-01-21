#pragma once
#include <3ds.h>

typedef struct
{
	u32 magic;
	u16 version;
	u16 reserved;
} smdhHeader_s;

typedef struct
{
	u16 shortDescription[0x40];
	u16 longDescription[0x80];
	u16 publisher[0x40];
} smdhTitle_s;

typedef struct
{
	u8 gameRatings[0x10];
	u32 regionLock;
	u8 matchMakerId[0xC];
	u32 flags;
	u16 eulaVersion;
	u16 reserved;
	u32 defaultFrame;
	u32 cecId;
} smdhSettings_s;

typedef struct
{
	smdhHeader_s header;
	smdhTitle_s applicationTitles[16];
	smdhSettings_s settings;
	u8 reserved[0x8];
	u8 smallIconData[0x480];
	u16 bigIconData[0x900];
} smdh_s;
