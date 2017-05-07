#pragma once

typedef enum
{
	imgId_appbubble,
	imgId_battery0,
	imgId_battery1,
	imgId_battery2,
	imgId_battery3,
	imgId_battery4,
	imgId_batteryCharge,
	imgId_bubble,
	imgId_defaultIcon,
	imgId_folderIcon,
	imgId_loading,
	imgId_logo,
	imgId_logo2,
	imgId_scrollbarKnob,
	imgId_scrollbarTrack,
	imgId_settings,
	imgId_wifi0,
	imgId_wifi1,
	imgId_wifi2,
	imgId_wifi3,
	imgId_wifiNull,

	MAX_IMAGES,
} ImageId;

typedef struct
{
	float width, height;
	struct
	{
		float left, top, right, bottom;
	} texcoord;
} imageInfo_s;

extern const imageInfo_s g_imageData[MAX_IMAGES];
