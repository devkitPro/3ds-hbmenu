#pragma once
#include <3ds.h>

typedef struct
{
	char* executable;
	char* descriptor;
	char* icon;
	char* arg;
	char* name;
	char* description;
	char* author;
} shortcut_s;

Result shortcutCreate(shortcut_s* d, const char* path);
void shortcutFree(shortcut_s* d);
