#include "common.h"
#include "parsing/shortcut.h"

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

	menu_s* menuFileAssoc = menuFileAssocGetCurrent();
	menuEntry_s* fileAssocEntry = NULL;

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
		bool fileAssocFlag = false;

		if (!found)
		{
			snprintf(tempbuf, sizeof(tempbuf)-1, "%.*s/%.*s.3dsx", sizeof(tempbuf)/2, me->path, sizeof(tempbuf)/2-7, name);
			found = fileExists(tempbuf);
		}
		LOG("Found: %d, File Assoc Entries %d", found, menuFileAssoc->nEntries);
		if (!found && menuFileAssoc->nEntries > 0) {
			fileAssocFlag = true;

			DIR* dir = opendir(fileAssocEntry->path);
			struct dirent* dp;

			if (dir) {
				int index = 0;
				while ((dp = readdir(dir))) {
					if (dp->d_name[0] == '.')
						continue;

					for (fileAssocEntry = menuFileAssoc->firstEntry, index = 0; fileAssocEntry; fileAssocEntry = fileAssocEntry->next, index++) {
						if (!fileAssocEntry->fileAssocType)
							continue;

						if (strcmp(dp->d_name, fileAssocEntry->fileAssocStr))
							continue;

						snprintf(tempbuf, sizeof(tempbuf) - 1, "%.*s/%.*s", (int)sizeof(tempbuf) / 2, me->path, (int)sizeof(tempbuf) / 2 - 7, dp->d_name);
						LOG("File Association: %s", tempbuf);
						found = fileExists(tempbuf);

						if (found)
							break;
					}
				}
				closedir(dir);
			}
		}

		if (found)
		{
			isAppBundleFolder = true;
			shortcut = false;
			me->type = fileAssocFlag ? ENTRY_TYPE_FILE_OTHER : ENTRY_TYPE_FILE;
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
			if (R_FAILED(shortcutCreate(&sc, me->path))) {
				LOG("Failed to create shortcut!");
				return false;
			}

			if (!fileExists(sc.executable))
			{
				LOG("Shortcut executable does not exist.");
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

	if (me->type == ENTRY_TYPE_FILE_OTHER) {
		if (menuFileAssoc->nEntries == 0)
			return false;

		int index = 0;
		char* strptr;

		for (fileAssocEntry = menuFileAssoc->firstEntry, index = 0; fileAssocEntry; fileAssocEntry = fileAssocEntry->next, index++) {
			if (!fileAssocEntry->fileAssocType)
				strptr = getExtension(me->path);

			if (fileAssocEntry->fileAssocType) {
				strptr = getSlash(me->path);
				if (strptr[0] == '/')
					strptr++;
			}

			if (strcmp(strptr, fileAssocEntry->fileAssocStr))
				continue;

			me->type = ENTRY_TYPE_FILE;

			/* attempt to load icon from entry->path (with extension t3x) */
			/* on failure, should use icon data from the entry */

			memset(tempbuf, 0, sizeof(tempbuf));
			strncpy(tempbuf, me->path, sizeof(tempbuf));
			tempbuf[sizeof(tempbuf) - 1] = 0;

			strptr = getExtension(tempbuf);
			strncpy(strptr, ".t3x", sizeof(tempbuf) - 1 - ((ptrdiff_t)strptr - (ptrdiff_t)tempbuf));

			bool iconLoaded = false;
			/* TODO: load an icon */

			/* attempt to load the smdh from entry->path with extension .smdh */
			/* on failure, use the config from the entry */
			memset(tempbuf, 0, sizeof(tempbuf));
			strncpy(tempbuf, me->path, sizeof(tempbuf));
			tempbuf[sizeof(tempbuf) - 1] = 0;

			strptr = getExtension(tempbuf);
			strncpy(strptr, ".smdh", sizeof(tempbuf) - 1 - ((ptrdiff_t)strptr - (ptrdiff_t)tempbuf));

			bool smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
			if (smdhLoaded) {
				menuEntryParseSmdh(me);
			} else {
				strncpy(me->author, fileAssocEntry->author, sizeof(me->author));
				me->author[sizeof(me->author) - 1] = 0;

				strncpy(me->description, fileAssocEntry->description, sizeof(me->description));
				me->description[sizeof(me->description) - 1] = 0;
			}

			/* initialize argument data */
			argData_s* argData = &me->args;
			argData_s* argDataAssoc = &fileAssocEntry->args;

			char* argSource = (char*)&argDataAssoc->buf[1];
			bool fTokenFound = false;

			argData->dst = (char*)&argData->buf[1];

			for (u32 argIndex = 0; argIndex < argDataAssoc->buf[0]; argIndex++, argSource += strlen(argSource) + 1) {
				if (argIndex) {
					strptr = strchr(argSource, '%');
					if (strptr && strptr[0] && strptr[1] && (strptr == argSource || strptr[-1] != "\\")) {
						if (strptr[1] == 'f') {
							memset(tempbuf, 0, sizeof(tempbuf));
							snprintf(tempbuf, sizeof(tempbuf) - 1, "%.*s%s%s", (int)((uintptr_t)strptr - (uintptr_t)argSource), argSource, me->path, &strptr[2]);

							launchAddArg(argData, tempbuf);
							fTokenFound = true;
							continue;
						}
					}
				}

				launchAddArg(argData, argSource);
			}

			if (!fTokenFound)
				launchAddArg(argData, me->path);

			strncpy(me->path, fileAssocEntry->path, sizeof(me->path));
			me->path[sizeof(me->path) - 1] = 0;

			return true;
		}

		return false;
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
