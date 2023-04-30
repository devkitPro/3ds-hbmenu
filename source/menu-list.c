#include "common.h"

static menu_s s_menu[2];
static menu_s s_menuFileAssoc[2];

static bool s_curMenu, s_curMenuFileAssoc;

menu_s* menuGetCurrent(void)
{
	return &s_menu[s_curMenu];
}

menu_s* menuFileAssocGetCurrent(void) {
	return &s_menuFileAssoc[s_curMenuFileAssoc];
}

menuEntry_s* menuCreateEntry(MenuEntryType type)
{
	menuEntry_s* me = (menuEntry_s*)malloc(sizeof(menuEntry_s));
	menuEntryInit(me, type);
	return me;
}

void menuDeleteEntry(menuEntry_s* me)
{
	menuEntryFree(me);
	free(me);
}

static void _menuAddEntry(menu_s* m, menuEntry_s* me)
{
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
	m->nEntries++;
}

static void _menuClear(menu_s* menu)
{
	menuEntry_s* cur, *next;
	for (cur = menu->firstEntry; cur; cur = next) {
		next = cur->next;
		menuDeleteEntry(cur);
	}
	memset(menu, 0, sizeof(*menu));
}

static void menuClear(void)
{
	_menuClear(&s_menu[!s_curMenu]);
}

void menuFileAssocClear(void)
{
	_menuClear(&s_menuFileAssoc[!s_curMenuFileAssoc]);
}

void menuFileAssocAddEntry(menuEntry_s* me)
{
	_menuAddEntry(&s_menuFileAssoc[!s_curMenuFileAssoc], me);
}

static void menuAddEntry(menuEntry_s* me)
{
	_menuAddEntry(&s_menu[!s_curMenu], me);
}

static int menuEntryCmp(const void *p1, const void *p2)
{
	const menuEntry_s* lhs = *(menuEntry_s**)p1;
	const menuEntry_s* rhs = *(menuEntry_s**)p2;

	if(lhs->isStarred != rhs->isStarred)
		return lhs->isStarred ? -1 : 1;
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

int menuFileAssocScan(const char* target)
{
	menuFileAssocClear();

	if (chdir(target) < 0)
		return 1;

	if (getcwd(s_menuFileAssoc[!s_curMenuFileAssoc].dirname, PATH_MAX + 1) == NULL)
		return 1;

	DIR* dir;
	struct dirent* dp;
	char temp[PATH_MAX + 1];

	dir = opendir(s_menuFileAssoc[!s_curMenuFileAssoc].dirname);
	if (!dir)
		return 2;

	while ((dp = readdir(dir))) {
		if (dp->d_name[0] == '.')
			continue;

		memset(temp, 0, sizeof(temp));
		snprintf(temp, sizeof(temp) - 1, "%s/%s", s_menuFileAssoc[!s_curMenuFileAssoc].dirname, dp->d_name);

		const char* ext = getExtension(dp->d_name);
		if (strcasecmp(ext, ".cfg") != 0)
			continue;

		menuEntryFileAssocLoad(temp);
	}

	closedir(dir);
	s_curMenuFileAssoc = !s_curMenuFileAssoc;
	menuFileAssocClear();

	return 0;
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
		archive_dir_t* dirSt = (archive_dir_t*)dir->dirData->dirStruct;
		FS_DirectoryEntry* entry = &dirSt->entry_data[dirSt->index];
		menuEntry_s* me = NULL;
		bool shortcut = false;
		if (entry->attributes & FS_ATTRIBUTE_HIDDEN || dp->d_name[0] == '.')
			continue;

		if (entry->attributes & FS_ATTRIBUTE_DIRECTORY)
			me = menuCreateEntry(ENTRY_TYPE_FOLDER);
		else
		{
			const char* ext = getExtension(dp->d_name);
			if (strcasecmp(ext, ".3dsx")==0 || (shortcut = strcasecmp(ext, ".xml")==0))
				me = menuCreateEntry(ENTRY_TYPE_FILE);

			if (!me)
				me = menuCreateEntry(ENTRY_TYPE_FILE_OTHER);
		}

		if (!me)
			continue;

		snprintf(me->path, sizeof(me->path), "%s/%s", s_menu[!s_curMenu].dirname, dp->d_name);
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

void menuToggleStar(menuEntry_s* me)
{
	me->isStarred = !me->isStarred;
	if (me->isStarred)
	{
		FILE* f = fopen(me->starpath, "w");
		if (f) fclose(f);
	} else
		remove(me->starpath);

	// Sort the menu again
	s_curMenu = !s_curMenu;
	menuSort();
	s_curMenu = !s_curMenu;
	menuClear();

	menu_s* menu = menuGetCurrent();
	int pos = -1;
	for (menuEntry_s* cur = menu->firstEntry; cur; cur = cur->next)
	{
		++pos;
		if (cur == me)
			break;
	}
	menu->curEntry = pos;
	menu->perturbed = true;
}
