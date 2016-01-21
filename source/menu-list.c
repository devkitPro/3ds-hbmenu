#include "menu.h"

static menu_s s_menu[2];
static bool s_curMenu;

menu_s* menuGetCurrent(void)
{
	return &s_menu[s_curMenu];
}

static menuEntry_s* menuCreateEntry(MenuEntryType type)
{
	menuEntry_s* me = (menuEntry_s*)malloc(sizeof(menuEntry_s));
	menuEntryInit(me, type);
	return me;
}

static void menuDeleteEntry(menuEntry_s* me)
{
	menuEntryFree(me);
	free(me);
}

static void menuAddEntry(menuEntry_s* me)
{
	menu_s* m = &s_menu[!s_curMenu];
	me->menu = m;
	if (m->lastEntry)
	{
		m->lastEntry->next = me;
		m->lastEntry = me;
	} else
	{
		m->firstEntry = me;
		m->lastEntry = me;
	}
	m->nEntries ++;
}

static void menuClear(void)
{
	menu_s* m = &s_menu[!s_curMenu];
	menuEntry_s *cur, *next;
	for (cur = m->firstEntry; cur; cur = next)
	{
		next = cur->next;
		menuDeleteEntry(cur);
	}
	memset(m, 0, sizeof(*m));
}

static int menuEntryCmp(const void *p1, const void *p2)
{
	const menuEntry_s* lhs = *(menuEntry_s**)p1;
	const menuEntry_s* rhs = *(menuEntry_s**)p2;

	if(lhs->type == rhs->type)
		return strcasecmp(lhs->name, rhs->name);
	if(lhs->type == ENTRY_TYPE_FOLDER)
		return -1;
	return 1;
}

static void menuSort(void)
{
	int i;
	menu_s* m = &s_menu[!s_curMenu];
	int nEntries = m->nEntries;
	if (nEntries==0) return;

	menuEntry_s** list = (menuEntry_s**)calloc(nEntries, sizeof(menuEntry_s*));
	if(list == NULL) return;

	menuEntry_s* p = m->firstEntry;
	for(i = 0; i < nEntries; ++i) {
		list[i] = p;
		p = p->next;
	}

	qsort(list, nEntries, sizeof(menuEntry_s*), menuEntryCmp);

	menuEntry_s** pp = &m->firstEntry;
	for(i = 0; i < nEntries; ++i) {
		*pp = list[i];
		pp = &(*pp)->next;
	}
	m->lastEntry = list[nEntries-1];
	*pp = NULL;

	free(list);
}

int menuScan(const char* target)
{
	if (chdir(target) < 0) return 1;
	getcwd(s_menu[!s_curMenu].dirname, PATH_MAX+1);

	DIR* dir;
	struct dirent* dp;
	dir = opendir(s_menu[!s_curMenu].dirname);
	if (!dir) return 2;

	while ((dp = readdir(dir)))
	{
		FS_DirectoryEntry* entry = &((sdmc_dir_t*)dir->dirData->dirStruct)->entry_data;
		menuEntry_s* me = NULL;
		bool shortcut = false;

		if (entry->attributes & FS_ATTRIBUTE_DIRECTORY)
			me = menuCreateEntry(ENTRY_TYPE_FOLDER);
		else
		{
			const char* ext = getExtension(dp->d_name);
			if (strcasecmp(ext, ".3dsx")==0 || (shortcut = strcasecmp(ext, ".xml")==0))
				me = menuCreateEntry(ENTRY_TYPE_FILE);
		}

		if (!me)
			continue;

		snprintf(me->path, sizeof(me->path), "%s%s", s_menu[!s_curMenu].dirname, dp->d_name);
		if (menuEntryLoad(me, dp->d_name, shortcut))
			menuAddEntry(me);
		else
			menuDeleteEntry(me);
	}

	closedir(dir);
	menuSort();

	// Swap the menu and clear the previous menu
	s_curMenu = !s_curMenu;
	menuClear();
	return 0;
}
