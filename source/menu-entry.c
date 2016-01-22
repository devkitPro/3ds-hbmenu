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
	return stat(path, &st)==0 && S_ISREG(st.st_mode);
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

bool menuEntryLoad(menuEntry_s* me, const char* name, bool shortcut)
{
	static char tempbuf[PATH_MAX+1];
	bool isOldAppFolder = false;

	strcpy(me->name, name);
	if (me->type == ENTRY_TYPE_FOLDER)
	{
		// Check for old style application folder
		snprintf(tempbuf, sizeof(tempbuf), "%s/boot.3dsx", me->path);
		bool found = fileExists(tempbuf);
		if (!found)
		{
			snprintf(tempbuf, sizeof(tempbuf), "%s/%s.3dsx", me->path, name);
			found = fileExists(tempbuf);
		}

		if (found)
		{
			isOldAppFolder = true;
			shortcut = false;
			me->type = ENTRY_TYPE_FILE;
			strcpy(me->path, tempbuf);
		} else
			strcpy(me->description, "Directory");
	}

	if (me->type == ENTRY_TYPE_FILE)
	{
		strcpy(me->name, name);
		strcpy(me->description, "Homebrew application");
		strcpy(me->author, "Unknown publisher");

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
			strcpy(me->path, sc.executable);
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

		if (!smdhLoaded) do
		{
			// Attempt loading external SMDH
			strcpy(tempbuf, me->path);
			char* ext = getExtension(tempbuf);
			strcpy(ext, ".smdh");
			smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
			if (smdhLoaded) break;

			strcpy(ext, ".icn");
			smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
			if (smdhLoaded) break;

			if (isOldAppFolder)
			{
				char* slash = getSlash(tempbuf);
				strcpy(slash, "/icon.smdh");
				smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
				if (smdhLoaded) break;

				strcpy(slash, "/icon.icn");
				smdhLoaded = menuEntryLoadExternalSmdh(me, tempbuf);
				if (smdhLoaded) break;
			}

			// Attempt loading the embedded SMDH
			smdhLoaded = menuEntryLoadEmbeddedSmdh(me);
			if (smdhLoaded) break;
		} while (0);

		if (smdhLoaded)
			menuEntryParseSmdh(me);

		// Metadata overrides for shortcuts
		if (shortcut)
		{
			if (sc.name) strncpy(me->name, sc.name, ENTRY_NAMELENGTH);
			if (sc.description) strncpy(me->description, sc.description, ENTRY_DESCLENGTH);
			if (sc.author) strncpy(me->author, sc.author, ENTRY_AUTHORLENGTH);
		}

		// Load the descriptor
		if (shortcut && fileExists(sc.descriptor))
			descriptorLoad(&me->descriptor, sc.descriptor);
		else
		{
			strcpy(tempbuf, me->path);
			strcpy(getExtension(tempbuf), ".xml");
			bool found = fileExists(tempbuf);
			if (!found && isOldAppFolder)
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

	safe_utf8_convert(me->name, me->smdh.applicationTitles[1].shortDescription, ENTRY_NAMELENGTH);
	safe_utf8_convert(me->description, me->smdh.applicationTitles[1].longDescription, ENTRY_NAMELENGTH);
	safe_utf8_convert(me->author, me->smdh.applicationTitles[1].publisher, ENTRY_NAMELENGTH);
}
