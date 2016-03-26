extern "C"
{
	#include "memmap.h"
}
#include <tinyxml2.h>

using namespace tinyxml2;

static u32 getXmlUnsignedInt(XMLElement* el)
{
	if(!el) return 0;

	const char* str = el->GetText();
	if(!str) return 0;

	return strtoul(str, NULL, 0);
}

static u32 getXmlInt(XMLElement* el)
{
	if(!el) return 0;

	const char* str = el->GetText();
	if(!str) return 0;

	return strtol(str, NULL, 0);
}

// TODO : error checking
memmap_t* memmapLoad(const char* path)
{
	if(!path)return NULL;

    XMLDocument doc;
    if(doc.LoadFile(path))return NULL;

    memmap_header_t header;
    XMLElement* header_element = doc.FirstChildElement("header");
	if(header_element)
	{
		header.num = getXmlUnsignedInt(header_element->FirstChildElement("num"));
		header.text_end = getXmlUnsignedInt(header_element->FirstChildElement("text_end"));
		header.data_address = getXmlUnsignedInt(header_element->FirstChildElement("data_address"));
		header.data_size = getXmlUnsignedInt(header_element->FirstChildElement("data_size"));
		header.processLinearOffset = getXmlUnsignedInt(header_element->FirstChildElement("processLinearOffset"));
		header.processHookAddress = getXmlUnsignedInt(header_element->FirstChildElement("processHookAddress"));
		header.processAppCodeAddress = getXmlUnsignedInt(header_element->FirstChildElement("processAppCodeAddress"));
		header.processHookTidLow = getXmlUnsignedInt(header_element->FirstChildElement("processHookTidLow"));
		header.processHookTidHigh = getXmlUnsignedInt(header_element->FirstChildElement("processHookTidHigh"));
		header.mediatype = getXmlUnsignedInt(header_element->FirstChildElement("mediatype"));
	}else return NULL;

	memmap_t* ret = (memmap_t*) malloc(sizeof(memmap_header_t) + header.num * sizeof(memmap_entry_t));
	if(!ret) return NULL;

	ret->header = header;

    XMLElement* map = doc.FirstChildElement("map");
    if(map)
    {
		u32 i = 0;

		for (tinyxml2::XMLElement* child = map->FirstChildElement(); child != NULL && i < header.num; child = child->NextSiblingElement())
		{
			if(!strcmp(child->Name(), "entry"))
			{
				ret->map[i].src = getXmlInt(child->FirstChildElement("src"));
				ret->map[i].dst = getXmlInt(child->FirstChildElement("dst"));
				ret->map[i].size = getXmlInt(child->FirstChildElement("size"));

				i++;
			}
		}

		if(i == header.num) return ret;
    }

    free(ret);

    return NULL;
}
