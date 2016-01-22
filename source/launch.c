#include "common.h"

static const loaderFuncs_s* s_loader;

void launchInit(void)
{
#define ADD_LOADER(_name) do \
	{ \
		extern const loaderFuncs_s _name; \
		if (_name.init()) \
		{ \
			s_loader = &_name; \
			return; \
		} \
	} while(0)

	ADD_LOADER(loader_Ninjhax1);
	ADD_LOADER(loader_Ninjhax2);

	// Shouldn't happen
	svcBreak(USERBREAK_PANIC);
}

void launchExit(void)
{
	s_loader->deinit();
}

const loaderFuncs_s* launchGetLoader(void)
{
	return s_loader;
}

void launchAddArg(argData_s* ad, const char* arg)
{
	size_t len = strlen(arg)+1;
	if ((ad->dst+len) >= (char*)(ad+1)) return; // Overflow
	ad->buf[0]++;
	strcpy(ad->dst, arg);
	ad->dst += len;
}

void launchAddArgsFromString(argData_s* ad, char* arg)
{
	char c, *pstr, *str=arg, *endarg = arg+strlen(arg);

	do
	{
		do
		{
			c = *str++;
		} while ((c == ' ' || c == '\t') && str < endarg);

		pstr = str-1;

		if (c == '\"')
		{
			pstr++;
			while(*str++ != '\"' && str < endarg);
		}
		else
		if (c == '\'')
		{
			pstr++;
			while(*str++ != '\'' && str < endarg);
		}
		else
		{
			do
			{
				c = *str++;
			} while (c != ' ' && c != '\t' && str < endarg);
		}

		str--;

		if (str == (endarg - 1))
		{
			if(*str == '\"' || *str == '\'') *(str++) = 0;
		}
		else
		{
			*(str++) = '\0';
		}

		launchAddArg(ad, pstr);

	} while(str<endarg);
}

void launchMenuEntry(menuEntry_s* me)
{
	if (me->descriptor.numTargetTitles)
	{
		// Not implemented - find the target title
		return;
	} else if (me->descriptor.selectTargetProcess)
	{
		// Not implemented - launch the title selector
		uiEnterState(UI_STATE_TITLESELECT);
		return;
	}

	// Scan the executable
	descriptorScanFile(&me->descriptor, me->path);

	// Launch it
	s_loader->launchFile(me->path, &me->args, &me->descriptor.executableMetadata);
}

Handle launchOpenFile(const char* path)
{
	if (strncmp(path, "sdmc:/", 6) == 0)
		path += 5;
	else if (*path != '/')
		return 0;

	// Convert the executable path to UTF-16
	static uint16_t __utf16path[PATH_MAX+1];
	ssize_t units = utf8_to_utf16(__utf16path, (const uint8_t*)path, PATH_MAX);
	if (units < 0 || units >= PATH_MAX) return 0;
	__utf16path[units] = 0;

	// Open the file directly
	FS_Archive arch = { ARCHIVE_SDMC, { PATH_EMPTY, 1, (u8*)"" }, 0 };
	FS_Path apath = { PATH_UTF16, (units+1)*2, (u8*)__utf16path };
	Handle file;
	Result res = FSUSER_OpenFileDirectly(&file, arch, apath, FS_OPEN_READ, 0);
	return R_SUCCEEDED(res) ? file : 0;
}
