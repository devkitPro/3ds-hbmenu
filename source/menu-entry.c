#include "common.h"
#include "parsing/shortcut.h"

#include <libconfig.h>

void menuEntryInit(menuEntry_s* me, MenuEntryType type)
{
	memset(me, 0, sizeof(*me));
	me->type = type;
	descriptorInit(&me->descriptor);
}

void menuEntryFree(menuEntry_s* me)
{
	descriptorFree(&me->descriptor);
	C3D_TexDelete(&me->texture);
}

bool fileExists(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

/*
* temporary solution for path issues
* credits: https://github.com/clibs/clib/blob/master/deps/path-normalize/path-normalize.c
*/

char* normalizePath(const char* path) {
	if (!path) return NULL;

	char *copy = strdup(path);
	if (NULL == copy) return NULL;

	char *ptr = copy;

	for (int i = 0; copy[i]; i++) {
		*ptr++ = path[i];
		if ('/' == path[i]) {
			i++;
			while ('/' == path[i]) i++;
			i--;
		}
	}

	*ptr = '\0';

	return copy;
}

static bool menuEntryLoadExternalIcon(menuEntry_s* me, const char* filepath) {
	if (true)
		return false;

	struct stat fileStat;

	if (stat(filepath, &fileStat) == -1)
		return false;

	FILE* icon = fopen(filepath, "rb");
	if (!icon)
		return false;

	if (!me->icon)
	{
		me->icon = &me->texture;
		C3D_TexInit(me->icon, 64, 64, GPU_RGB565);
	}

	int fileNo = fileno(icon);
	Tex3DS_Texture tex = Tex3DS_TextureImportFD(fileNo, me->icon, NULL, true);

	// Copy 48x48 -> 64x64
	u16* dest = (u16*)me->icon->data + (64-48)*64;
	// u16* src = (u16*)data;

	int j;
	for (j = 0; j < 48; j += 8)
	{
		// memcpy(dest, src, 48*8*sizeof(u16));
		// src += 48*8;
		dest += 64*8;
	}
}

static bool menuEntryLoadEmbeddedSmdh(menuEntry_s* me)
{
	_3DSX_Header header;

	FILE* f = fopen(me->path, "rb");
	if (!f) return false;

	if (fread(&header, sizeof(header), 1, f) != 1
		|| header.headerSize < sizeof(header)
		|| header.smdhSize < sizeof(smdh_s))
	{
		fclose(f);
		return false;
	}

	fseek(f, header.smdhOffset, SEEK_SET);
	bool ok = fread(&me->smdh, sizeof(smdh_s), 1, f) == 1;
	fclose(f);
	return ok;
}

static bool menuEntryLoadExternalSmdh(menuEntry_s* me, const char* file)
{
	FILE* f = fopen(file, "rb");
	if (!f) return false;
	bool ok = fread(&me->smdh, sizeof(smdh_s), 1, f) == 1;
	fclose(f);
	return ok;
}

void menuEntryFileAssocLoad(const char* filepath)
{
	bool success = false;
	bool iconLoaded = false;

	menuEntry_s entry;

	config_setting_t *fileAssoc = NULL;
	config_setting_t *targets = NULL;
	config_setting_t *target = NULL;
	config_setting_t *appArguments = NULL;
	config_setting_t *targetArguments = NULL;

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

	uint8_t *iconGraphic = NULL;
	uint8_t *iconGraphicSmall = NULL;

	const char *stringValue = NULL;

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
					menuEntryInit(&entry, ENTRY_TYPE_FILE);

					strncpy(entry.path, appPath, sizeof(entry.path) - 1);
					entry.path[sizeof(entry.path) - 1] = 0;

					stringValue = getSlash(appPath);
					if (stringValue[0] == '/')
						stringValue++;

					if (menuEntryLoad(&entry, stringValue, true)) {
						strncpy(appAuthor, entry.author, sizeof(appAuthor));
						appAuthor[sizeof(appAuthor) - 1] = 0;

						strncpy(appDescription, entry.description, sizeof(appDescription));
						appDescription[sizeof(appDescription) - 1] = 0;

						// iconGraphic = entry->icon;
						// iconGraphicSmall = entry->ic
						success = true;
					}

					menuEntryFree(&entry);
				}
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

					if (config_setting_lookup_string(target, "file_extension", &stringValue))
						strncpy(targetFileExtension, stringValue, sizeof(targetFileExtension) - 1);

					if (config_setting_lookup_string(target, "filename", &stringValue))
						strncpy(targetFilename, stringValue, sizeof(targetFilename) - 1);

					targetArguments = config_setting_lookup(target, "app_args");

					if ((targetFileExtension[0] != 0) == (targetFilename[0] != 0))
						continue;

					menuEntryInit(&entry, ENTRY_TYPE_FILEASSOC);
					iconLoaded = false;

					strncpy(entry.path, appPath, sizeof(entry.path));
					entry.path[sizeof(entry.path) - 1] = 0;

					strncpy(entry.author, appAuthor, sizeof(entry.author));
					entry.author[sizeof(entry.author) - 1] = 0;

					strncpy(entry.description, appDescription, sizeof(entry.description));
					entry.description[sizeof(entry.description) - 1] = 0;

					if (targetFileExtension[0]) {
						entry.fileAssocType = 0;
						strncpy(entry.fileAssocStr, targetFileExtension, sizeof(entry.fileAssocStr));
					} else if (targetFilename[0]) {
						entry.fileAssocType = 1;
						strncpy(entry.fileAssocStr, targetFilename, sizeof(entry.fileAssocStr));
					}

					entry.fileAssocStr[sizeof(entry.fileAssocStr) - 1] = 0;

					// if (targetIconPath[0])
					// 	iconLoaded = menuEntryLoadExternalIcon(entry, targetIconPath);

					// if (!iconLoaded && mainIconPath[0])
					// 	iconLoaded = menuEntryLoadExternalIcon(entry, mainIconPath);
					argData_s* argData = &entry.args;
					argData->dst = (char*)&argData->buf[1];

					launchAddArg(argData, entry.path);

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

					menuFileAssocAddEntry(&entry);
				}
			}
		}
	}

	config_destroy(&config);
}

static void fixSpaceNewLine(char* buf)
{
	char *outp = buf, *inp = buf;
	char lastc = 0;
	do
	{
		char c = *inp++;
		if (c == ' ' && lastc == ' ')
			outp[-1] = '\n';
		else
			*outp++ = c;
		lastc = c;
	} while (lastc);
}

bool menuEntryLoad(menuEntry_s* me, const char* name, bool shortcut)
{
	static char tempbuf[PATH_MAX+1];
	bool isAppBundleFolder = false;

	tempbuf[PATH_MAX] = 0;
	strcpy(me->name, name);
	snprintf(me->starpath, sizeof(me->starpath)-1, "%.*s.star", sizeof(me->starpath)-12, me->path);

	if (me->type == ENTRY_TYPE_FOLDER) do
	{
		// Check if this folder is an application bundle (except if it's the starting directory)
		if (strcmp(me->path, "sdmc:/3ds") == 0)
			break;

		snprintf(tempbuf, sizeof(tempbuf)-1, "%.*s/boot.3dsx", sizeof(tempbuf)-12, me->path);
		bool found = fileExists(tempbuf);
		if (!found)
		{
			snprintf(tempbuf, sizeof(tempbuf)-1, "%.*s/%.*s.3dsx", sizeof(tempbuf)/2, me->path, sizeof(tempbuf)/2-7, name);
			found = fileExists(tempbuf);
		}

		if (found)
		{
			isAppBundleFolder = true;
			shortcut = false;
			me->type = ENTRY_TYPE_FILE;
			strcpy(me->path, tempbuf);
		}
	} while (0);

	if (me->type == ENTRY_TYPE_FOLDER)
		strcpy(me->description, textGetString(StrId_Directory));

	if (me->type == ENTRY_TYPE_FILE)
	{
		strcpy(me->name, name);
		strcpy(me->description, textGetString(StrId_DefaultLongTitle));
		strcpy(me->author, textGetString(StrId_DefaultPublisher));

		shortcut_s sc;

		if (shortcut)
		{
			if (R_FAILED(shortcutCreate(&sc, me->path)))
				return false;
			if (!fileExists(sc.executable))
			{
				shortcutFree(&sc);
				return false;
			}
			strcpy(me->path, "sdmc:");
			strcat(me->path, sc.executable);
		}

		bool smdhLoaded = false;

		// Load the SMDH
		if (shortcut)
		{
			FILE* f = sc.icon ? fopen(sc.icon, "rb") : NULL;
			if (f)
			{
				smdhLoaded = fread(&me->smdh, sizeof(smdh_s), 1, f) == 1;
				fclose(f);
			}
		}

		if (!smdhLoaded)
			// Attempt loading the embedded SMDH
			smdhLoaded = menuEntryLoadEmbeddedSmdh(me);

		if (!smdhLoaded && isAppBundleFolder) do
		{
			// Attempt loading external SMDH from app bundle folder
			strcpy(tempbuf, me->path);
			char* ext = getExtension(tempbuf);
			strcpy(ext, ".smdh");
			smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
			if (smdhLoaded) break;

			char* slash = getSlash(tempbuf);
			strcpy(slash, "/icon.smdh");
			smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
			if (smdhLoaded) break;
		} while (0);

		if (smdhLoaded)
		{
			menuEntryParseSmdh(me);

			// Detect HANS, and only show it if the loader supports target titles
			if (strcmp(me->name, "HANS")==0 && !loaderCanUseTitles())
				return false;

			// Fix description for some applications using multiple spaces to indicate newline
			fixSpaceNewLine(me->description);
		}

		// Metadata overrides for shortcuts
		if (shortcut)
		{
			if (sc.name) strncpy(me->name, sc.name, ENTRY_NAMELENGTH);
			if (sc.description) strncpy(me->description, sc.description, ENTRY_DESCLENGTH);
			if (sc.author) strncpy(me->author, sc.author, ENTRY_AUTHORLENGTH);
		}

		// Load the descriptor
		if (shortcut && sc.descriptor && fileExists(sc.descriptor))
			descriptorLoad(&me->descriptor, sc.descriptor);
		else
		{
			strcpy(tempbuf, me->path);
			strcpy(getExtension(tempbuf), ".xml");
			bool found = fileExists(tempbuf);
			if (!found && isAppBundleFolder)
			{
				strcpy(tempbuf, me->path);
				strcpy(getSlash(tempbuf), "/descriptor.xml");
				found = fileExists(tempbuf);
			}
			if (found)
				descriptorLoad(&me->descriptor, tempbuf);
		}

		// Initialize the argument data
		argData_s* ad = &me->args;
		ad->dst = (char*)&ad->buf[1];
		launchAddArg(ad, me->path);

		// Load the argument(s) from the shortcut
		if (shortcut && sc.arg && *sc.arg)
			launchAddArgsFromString(ad, sc.arg);

		if (shortcut)
			shortcutFree(&sc);
	}

	me->isStarred = me->type == ENTRY_TYPE_FILE && fileExists(me->starpath);
	return true;
}

static void safe_utf8_convert(char* buf, const u16* input, size_t bufsize)
{
	ssize_t units = utf16_to_utf8((uint8_t*)buf, input, bufsize);
	if (units < 0) units = 0;
	buf[units] = 0;
}

void menuEntryParseSmdh(menuEntry_s* me)
{
	if (!me->icon)
	{
		me->icon = &me->texture;
		C3D_TexInit(me->icon, 64, 64, GPU_RGB565);
	}

	// Copy 48x48 -> 64x64
	u16* dest = (u16*)me->icon->data + (64-48)*64;
	u16* src = (u16*)me->smdh.bigIconData;
	int j;
	for (j = 0; j < 48; j += 8)
	{
		memcpy(dest, src, 48*8*sizeof(u16));
		src += 48*8;
		dest += 64*8;
	}

	int lang = textGetLang();
	safe_utf8_convert(me->name, me->smdh.applicationTitles[lang].shortDescription, ENTRY_NAMELENGTH);
	safe_utf8_convert(me->description, me->smdh.applicationTitles[lang].longDescription, ENTRY_NAMELENGTH);
	safe_utf8_convert(me->author, me->smdh.applicationTitles[lang].publisher, ENTRY_NAMELENGTH);
}
