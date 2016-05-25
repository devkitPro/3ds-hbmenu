#include "common.h"

extern void (*__system_retAddr)(void);

typedef struct
{
	// Mandatory fields
	const char* name;
	bool (* init)(void);
	void (* deinit)(void);
	void (* launchFile)(const char* path, argData_s* args, executableMetadata_s* em);

	// Optional fields
	void (* useTitle)(u64 tid, u8 mediatype);
} loaderFuncs_s;

void launchInit(void);
void launchExit(void);
const loaderFuncs_s* launchGetLoader(void);
size_t launchAddArg(argData_s* ad, const char* arg);
void launchAddArgsFromString(argData_s* ad, char* arg);
void launchMenuEntry(menuEntry_s* me);
Handle launchOpenFile(const char* path);
bool launchHomeMenuEnabled(void);
void launchHomeMenu(void);
