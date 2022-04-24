#include "common.h"

#include <libconfig.h>

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

static void _menuAddEntry(menu_s* m, menuEntry_s* me)
{
	LOG("Adding %s: %s", me->name, me->description);
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

void menuEntryFileAssocLoad(const char* filepath)
{
	bool success = false;
	bool iconLoaded = false;

	menuEntry_s* entry = NULL;

	config_setting_t* fileAssoc = NULL;
	config_setting_t* targets = NULL;
	config_setting_t* target = NULL;
	config_setting_t* appArguments = NULL;
	config_setting_t* targetArguments = NULL;

	config_t config = {0};

	int targetsLength = 0;
	int argsLength = 0;
	int index = 0;

	char appPath[PATH_MAX + 8];
	char mainIconPath[PATH_MAX + 1];
	char targetIconPath[PATH_MAX + 1];
	char targetFileExtension[PATH_MAX + 1];
	char targetFilename[PATH_MAX + 1];

	char appAuthor[ENTRY_AUTHORLENGTH + 2];
	char appDescription[ENTRY_DESCLENGTH + 2];

	uint8_t* iconGraphic = NULL;
	uint8_t* iconGraphicSmall = NULL;

	const char* stringValue = NULL;

	config_init(&config);

	memset(appPath, 0, sizeof(appPath));
	memset(mainIconPath, 0, sizeof(mainIconPath));
	memset(appAuthor, 0, sizeof(appAuthor));
	memset(appDescription, 0, sizeof(appDescription));


	if (!fileExists(filepath))
		return;

	if (config_read_file(&config, filepath)) {

		fileAssoc = config_lookup(&config, "fileassoc");

		if (fileAssoc != NULL) {
			if (config_setting_lookup_string(fileAssoc, "app_path", &stringValue))
				snprintf(appPath, sizeof(appPath) - 1, "%s%s", menuGetRootBasePath(), stringValue);

			if (config_setting_lookup_string(fileAssoc, "icon_path", &stringValue))
                snprintf(mainIconPath, sizeof(mainIconPath) - 1, "%s%s", menuGetRootBasePath(), stringValue);

			appArguments = config_setting_lookup(fileAssoc, "app_args");
            targets = config_setting_lookup(fileAssoc, "targets");

			if (appPath[0] && targets) {
				targetsLength = config_setting_length(targets);

				if (targetsLength > 0) {
					entry = menuCreateEntry(ENTRY_TYPE_FILE);
					success = false;

					if (entry) {
						strncpy(entry->path, appPath, sizeof(entry->path) - 1);
						entry->path[sizeof(entry->path) - 1] = 0;
						LOG("App Path: %s", entry->path);

						stringValue = getSlash(appPath);
						LOG("Value: %s", stringValue);
						if (stringValue[0] == '/')
							stringValue++;
						LOG("Value: %s", stringValue);
						if (menuEntryLoad(entry, stringValue, false)) {
							LOG("Loaded Entry!");
							strncpy(appAuthor, entry->author, sizeof(appAuthor));
							appAuthor[sizeof(appAuthor) - 1] = 0;
							LOG("App Author: %s", appAuthor);
							strncpy(appDescription, entry->description, sizeof(appDescription));
							appDescription[sizeof(appDescription) - 1] = 0;
							LOG("App Description: %s", appDescription);
							/* TODO: load icon */
							// iconGraphic = entry->icon;
							// iconGraphicSmall = entry->ic

							success = true;
						}

						menuDeleteEntry(entry);
						entry = NULL;
					}

					if (success) {
						for (index = 0; index < targetsLength; index++) {
							target = config_setting_get_elem(targets, index);

							if (target == NULL)
								continue;

							memset(targetIconPath, 0, sizeof(targetIconPath));
							memset(targetFileExtension, 0, sizeof(targetFileExtension));
							memset(targetFilename, 0, sizeof(targetFilename));

							if (config_setting_lookup_string(target, "icon_path", &stringValue))
								snprintf(targetIconPath, sizeof(targetIconPath) - 1, "%s%s", menuGetRootBasePath(), stringValue);
							LOG("Target Icon: %s", targetIconPath == NULL ? "None" : targetIconPath);
							if (config_setting_lookup_string(target, "file_extension", &stringValue))
								strncpy(targetFileExtension, stringValue, sizeof(targetFileExtension) - 1);
							LOG("Target Extension: %s", targetFileExtension == NULL ? "None" : targetFileExtension);
							if (config_setting_lookup_string(target, "filename", &stringValue))
								strncpy(targetFilename, stringValue, sizeof(targetFilename) - 1);
							LOG("Target Filename: %s", targetFilename == NULL ? "None" : targetFilename);
							targetArguments = config_setting_lookup(target, "app_args");

							if ((targetFileExtension[0] != 0) == (targetFilename[0] != 0))
								continue;

							entry = menuCreateEntry(ENTRY_TYPE_FILEASSOC);
							iconLoaded = false;

							if (entry) {
								strncpy(entry->path, appPath, sizeof(entry->path));
								entry->path[sizeof(entry->path) - 1] = 0;

								strncpy(entry->author, appAuthor, sizeof(entry->author));
								entry->author[sizeof(entry->author) - 1] = 0;

								strncpy(entry->description, appDescription, sizeof(entry->description));
								entry->description[sizeof(entry->description) - 1] = 0;

								if (targetFileExtension[0]) {
									entry->fileAssocType = 0;
									strncpy(entry->fileAssocStr, targetFileExtension, sizeof(entry->fileAssocStr));
								} else if (targetFilename[0]) {
									entry->fileAssocType = 1;
									strncpy(entry->fileAssocStr, targetFilename, sizeof(entry->fileAssocStr));
								}

								entry->fileAssocStr[sizeof(entry->fileAssocStr) - 1] = 0;

								// if (targetIconPath[0])
								// 	iconLoaded = menuEntryLoadExternalIcon(entry, targetIconPath);

								// if (!iconLoaded && mainIconPath[0])
								// 	iconLoaded = menuEntryLoadExternalIcon(entry, mainIconPath);
								argData_s* argData = &entry->args;
								argData->dst = (char*)&argData->buf[1];

								launchAddArg(argData, entry->path);

								config_setting_t* configArgs = targetArguments ? targetArguments : appArguments;
								if (configArgs) {
									argsLength = config_setting_length(configArgs);
									for (int argument = 0; argument < argsLength; argument++) {
										stringValue = config_setting_get_string_elem(configArgs, argument);

										if (stringValue == NULL)
											continue;

										launchAddArg(argData, stringValue);
									}
								}
							}

							if (entry)
								menuFileAssocAddEntry(entry);
						}
					}
				}
			}
		}
	}

	config_destroy(&config);
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

		menuEntryFileAssocLoad(normalizePath(temp));
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
