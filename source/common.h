#pragma once

// C stdlib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

// 3DS includes
#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>

// hbmenu includes
#include "ui.h"
#include "drawing.h"
#include "text.h"
#include "worker.h"
#include "menu.h"
#include "launch.h"
#include "titles.h"
#include "ui/error.h"

#define DIRECTORY_SEPARATOR_CHAR '/'
static const char DIRECTORY_SEPARATOR[] = "/";
