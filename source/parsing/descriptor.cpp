extern "C"
{
	#include "descriptor.h"
}
#include <tinyxml2.h>

using namespace tinyxml2;

void descriptorInit(descriptor_s* d)
{
	if(!d)return;

	d->targetTitles = NULL;
	d->numTargetTitles = 0;

	d->requestedServices = NULL;
	d->numRequestedServices = 0;

	d->selectTargetProcess = false;
	d->autodetectServices = true;

	scannerInit(&d->executableMetadata);
}

// TODO : error checking
void descriptorLoad(descriptor_s* d, const char* path)
{
	if(!d || !path)return;

	XMLDocument doc;
	if(doc.LoadFile(path))return;

	XMLElement* targets = doc.FirstChildElement("targets");
	if(targets)
	{
		// grab selectable target flag (default to false)
		{
			if(targets->QueryBoolAttribute("selectable", &d->selectTargetProcess)) d->selectTargetProcess = false;
		}

		// grab preferred target titles
		{
			d->numTargetTitles = 0;
			for (tinyxml2::XMLElement* child = targets->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
			{
				if(!strcmp(child->Name(), "title"))
				{
					d->numTargetTitles++;
				}
			}

			d->targetTitles = (targetTitle_s*)malloc(sizeof(targetTitle_s) * d->numTargetTitles);
			d->numTargetTitles = 0;

			for (tinyxml2::XMLElement* child = targets->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
			{
				if(!strcmp(child->Name(), "title"))
				{
					// SD is default mediatype
					int mediatype;
					if(child->QueryIntAttribute("mediatype", &mediatype))mediatype = 1;

					d->targetTitles[d->numTargetTitles].tid = strtoull(child->GetText(), NULL, 16);
					d->targetTitles[d->numTargetTitles].mediatype = mediatype;

					d->numTargetTitles++;
				}
			}
		}
    }

	XMLElement* services = doc.FirstChildElement("services");
	if(services)
	{
		// grab "autodetect services" flag (default to true)
		{
			if(services->QueryBoolAttribute("autodetect", &d->autodetectServices)) d->autodetectServices = true;
		}

		// grab requested services
		{
			d->numRequestedServices = 0;
			for (tinyxml2::XMLElement* child = services->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
			{
				if(!strcmp(child->Name(), "request"))
				{
					d->numRequestedServices++;
				}
			}

			d->requestedServices = (serviceRequest_s*)malloc(sizeof(serviceRequest_s) * d->numRequestedServices);
			d->numRequestedServices = 0;

			for (tinyxml2::XMLElement* child = services->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
			{
				if(!strcmp(child->Name(), "request"))
				{
					serviceRequest_s* req = &d->requestedServices[d->numRequestedServices];

					// 1 (highest) is default priority
					if(child->QueryIntAttribute("priority", &req->priority)) req->priority = 1;

					strncpy(req->name, child->GetText(), 8);
					req->name[8] = 0;

					d->numRequestedServices++;
				}
			}
		}
	}
}

void descriptorFree(descriptor_s* d)
{
	if(!d)return;

	if(d->targetTitles)
	{
		free(d->targetTitles);
		d->targetTitles = NULL;
	}

	if(d->requestedServices)
	{
		free(d->requestedServices);
		d->requestedServices = NULL;
	}
}
