#pragma once
#include "scanner.h"

typedef struct
{
	u64 tid;
	u8 mediatype;
} targetTitle_s;

typedef struct
{
	char name[9];
	int priority;
} serviceRequest_s;

typedef struct
{
	targetTitle_s* targetTitles;
	u32 numTargetTitles;

	serviceRequest_s *requestedServices;
	u32 numRequestedServices;

	bool selectTargetProcess;
	bool autodetectServices;

	executableMetadata_s executableMetadata;
} descriptor_s;

void descriptorInit(descriptor_s* d);
void descriptorFree(descriptor_s* d);
void descriptorLoad(descriptor_s* d, const char* path);
