#pragma once
#include "common.h"
#include "parsing/smdh.h"
#include "parsing/descriptor.h"

#define ENTRY_NAMELENGTH   (0x40*3)
#define ENTRY_DESCLENGTH   (0x80*3)
#define ENTRY_AUTHORLENGTH (0x40*3)
#define ENTRY_ARGBUFSIZE   0x400

typedef enum
{
	ENTRY_TYPE_FILE,
	ENTRY_TYPE_FOLDER,
	ENTRY_TYPE_FILEASSOC,
	ENTRY_TYPE_FILE_OTHER
} MenuEntryType;

typedef struct menuEntry_s_tag menuEntry_s;
typedef struct menu_s_tag menu_s;

typedef struct
{
	char* dst;
	u32 buf[ENTRY_ARGBUFSIZE/sizeof(u32)];
} argData_s;

struct menuEntry_s_tag
{
	menu_s* menu;
	menuEntry_s* next;
	MenuEntryType type;

	char path[PATH_MAX+1];
	char starpath[PATH_MAX+1];
	argData_s args;

	char name[ENTRY_NAMELENGTH+1];
	char description[ENTRY_DESCLENGTH+1];
	char author[ENTRY_AUTHORLENGTH+1];

    bool fileAssocType; 			//< 0 file_extension, 1 = filename
    char fileAssocStr[PATH_MAX+1];	//< file_extension/filename

	smdh_s smdh;
	descriptor_s descriptor;

	C3D_Tex* icon;
	C3D_Tex texture;

	u64 titleId;
	u8 titleMediatype;
	bool titleSelected;

	bool isStarred;
};

void menuEntryInit(menuEntry_s* me, MenuEntryType type);
void menuEntryFree(menuEntry_s* me);
bool menuEntryLoad(menuEntry_s* me, const char* name, bool shortcut);
void menuEntryParseSmdh(menuEntry_s* me);
void menuEntryFileAssocLoad(const char* filepath);

struct menu_s_tag
{
	menuEntry_s *firstEntry, *lastEntry;
	int nEntries;
	int curEntry;

	char dirname[PATH_MAX+1];

	float scrollTarget;
	float scrollLocation;
	float scrollVelocity;

	touchPosition previousTouch, firstTouch;
	int touchTimer;
	bool perturbed;
};

menu_s* menuGetCurrent(void);
int menuScan(const char* target);
char *menuGetRootBasePath(void);
void menuStartupPath(void);
void menuToggleStar(menuEntry_s* me);

menu_s* menuFileAssocGetCurrent(void);
void menuFileAssocClear(void);
int menuFileAssocScan(const char* target);
void menuFileAssocAddEntry(menuEntry_s* me);

char* normalizePath(const char* path);

static inline char* getExtension(const char* str)
{
	const char* p;
	for (p = str+strlen(str); p >= str && *p != '.'; p--);
	return (char*)p;
}

static inline char* getSlash(const char* str)
{
	const char* p;
	for (p = str+strlen(str); p >= str && *p != '/'; p--);
	return (char*)p;
}
