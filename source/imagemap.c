#include "imagemap.h"

const imageInfo_s g_imageData[MAX_IMAGES] =
{
	[imgId_appbubble] = { .width = 294.0f, .height = 63.0f, .texcoord = { .left = 0.0f/512, .top = 256.0f/256, .right = 294.0f/512, .bottom = 193.0f/256, }, },
	[imgId_battery0] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 0.0f/512, .top = 29.0f/256, .right = 27.0f/512, .bottom = 11.0f/256, }, },
	[imgId_battery1] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 28.0f/512, .top = 29.0f/256, .right = 55.0f/512, .bottom = 11.0f/256, }, },
	[imgId_battery2] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 56.0f/512, .top = 29.0f/256, .right = 83.0f/512, .bottom = 11.0f/256, }, },
	[imgId_battery3] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 98.0f/512, .top = 44.0f/256, .right = 125.0f/512, .bottom = 26.0f/256, }, },
	[imgId_battery4] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 137.0f/512, .top = 78.0f/256, .right = 164.0f/512, .bottom = 60.0f/256, }, },
	[imgId_batteryCharge] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 295.0f/512, .top = 223.0f/256, .right = 322.0f/512, .bottom = 205.0f/256, }, },
	[imgId_bubble] = { .width = 32.0f, .height = 32.0f, .texcoord = { .left = 295.0f/512, .top = 256.0f/256, .right = 327.0f/512, .bottom = 224.0f/256, }, },
	[imgId_defaultIcon] = { .width = 48.0f, .height = 48.0f, .texcoord = { .left = 0.0f/512, .top = 78.0f/256, .right = 48.0f/512, .bottom = 30.0f/256, }, },
	[imgId_folderIcon] = { .width = 48.0f, .height = 48.0f, .texcoord = { .left = 49.0f/512, .top = 78.0f/256, .right = 97.0f/512, .bottom = 30.0f/256, }, },
	[imgId_loading] = { .width = 8.0f, .height = 8.0f, .texcoord = { .left = 84.0f/512, .top = 29.0f/256, .right = 92.0f/512, .bottom = 21.0f/256, }, },
	[imgId_logo] = { .width = 271.0f, .height = 113.0f, .texcoord = { .left = 0.0f/512, .top = 192.0f/256, .right = 271.0f/512, .bottom = 79.0f/256, }, },
	[imgId_scrollbarKnob] = { .width = 7.0f, .height = 57.0f, .texcoord = { .left = 295.0f/512, .top = 204.0f/256, .right = 302.0f/512, .bottom = 147.0f/256, }, },
	[imgId_scrollbarTrack] = { .width = 7.0f, .height = 200.0f, .texcoord = { .left = 349.0f/512, .top = 256.0f/256, .right = 356.0f/512, .bottom = 56.0f/256, }, },
	[imgId_settings] = { .width = 38.0f, .height = 33.0f, .texcoord = { .left = 98.0f/512, .top = 78.0f/256, .right = 136.0f/512, .bottom = 45.0f/256, }, },
	[imgId_wifi0] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 328.0f/512, .top = 256.0f/256, .right = 348.0f/512, .bottom = 238.0f/256, }, },
	[imgId_wifi1] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 137.0f/512, .top = 59.0f/256, .right = 157.0f/512, .bottom = 41.0f/256, }, },
	[imgId_wifi2] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 272.0f/512, .top = 192.0f/256, .right = 292.0f/512, .bottom = 174.0f/256, }, },
	[imgId_wifi3] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 165.0f/512, .top = 78.0f/256, .right = 185.0f/512, .bottom = 60.0f/256, }, },
	[imgId_wifiNull] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 328.0f/512, .top = 237.0f/256, .right = 348.0f/512, .bottom = 219.0f/256, }, },
};
