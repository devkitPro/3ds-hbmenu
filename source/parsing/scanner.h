#pragma once
#include <3ds.h>
#include "3dsx.h"
#define NUM_SERVICESTHATMATTER 5

typedef struct
{
	bool scanned;
	u32 sectionSizes[3];
	bool servicesThatMatter[NUM_SERVICESTHATMATTER];
} executableMetadata_s;

void scannerInit(executableMetadata_s* em);
void scannerScan(executableMetadata_s* em, const char* path);
