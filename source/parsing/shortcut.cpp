extern "C"
{
	#include "shortcut.h"
}
#include <tinyxml2.h>

using namespace tinyxml2;

static void loadXmlString(char** out, XMLElement* in, const char* key)
{
	if (!out || !in || !key)return;

	XMLElement* node = in->FirstChildElement(key);
	if (node)
	{
		const char* str = node->GetText();
		if (str)
			*out = strdup(str);
	}
}

// TODO : error checking
static Result shortcutLoad(shortcut_s* s, const char* path)
{
	XMLDocument doc;
	if(doc.LoadFile(path))return -2;

	XMLElement* shortcut = doc.FirstChildElement("shortcut");
	if(shortcut)
	{
		XMLElement* executable = shortcut->FirstChildElement("executable");
		if(executable)
		{
			const char* str = executable->GetText();
			if(str)
				s->executable = strdup(str);
		}
		if(!s->executable) return -3;

		XMLElement* descriptor = shortcut->FirstChildElement("descriptor");
		const char* descriptor_path = path;
		if(descriptor) descriptor_path = descriptor->GetText();
		if(descriptor_path)
			s->descriptor = strdup(descriptor_path);

		loadXmlString(&s->icon, shortcut, "icon");
		loadXmlString(&s->arg, shortcut, "arg");
		loadXmlString(&s->name, shortcut, "name");
		loadXmlString(&s->description, shortcut, "description");
		loadXmlString(&s->author, shortcut, "author");
	}else return -4;

	return 0;
}

Result shortcutCreate(shortcut_s* s, const char* path)
{
	memset(s, 0, sizeof(*s));
	Result res = shortcutLoad(s, path);
	if (R_FAILED(res))
		shortcutFree(s);
	return res;
}

void shortcutFree(shortcut_s* s)
{
	if (s->executable) free(s->executable);
	if (s->descriptor) free(s->descriptor);
	if (s->icon) free(s->icon);
	if (s->arg) free(s->arg);
	if (s->name) free(s->name);
	if (s->description) free(s->description);
	if (s->author) free(s->author);
}
