#include "imagemap.h"

const imageInfo_s g_imageData[MAX_IMAGES] =
{
	[imgId_appbubble] = { .width = 294.0f, .height = 63.0f, .texcoord = { .left = 0.0f/512, .top = 512.0f/512, .right = 294.0f/512, .bottom = 449.0f/512, }, },
	[imgId_battery0] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 49.0f/512, .top = 220.0f/512, .right = 76.0f/512, .bottom = 202.0f/512, }, },
	[imgId_battery1] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 344.0f/512, .top = 512.0f/512, .right = 371.0f/512, .bottom = 494.0f/512, }, },
	[imgId_battery2] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 49.0f/512, .top = 201.0f/512, .right = 76.0f/512, .bottom = 183.0f/512, }, },
	[imgId_battery3] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 344.0f/512, .top = 493.0f/512, .right = 371.0f/512, .bottom = 475.0f/512, }, },
	[imgId_battery4] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 77.0f/512, .top = 220.0f/512, .right = 104.0f/512, .bottom = 202.0f/512, }, },
	[imgId_batteryCharge] = { .width = 27.0f, .height = 18.0f, .texcoord = { .left = 272.0f/512, .top = 414.0f/512, .right = 299.0f/512, .bottom = 396.0f/512, }, },
	[imgId_bubble] = { .width = 32.0f, .height = 32.0f, .texcoord = { .left = 0.0f/512, .top = 171.0f/512, .right = 32.0f/512, .bottom = 139.0f/512, }, },
	[imgId_defaultIcon] = { .width = 48.0f, .height = 48.0f, .texcoord = { .left = 0.0f/512, .top = 220.0f/512, .right = 48.0f/512, .bottom = 172.0f/512, }, },
	[imgId_folderIcon] = { .width = 48.0f, .height = 48.0f, .texcoord = { .left = 295.0f/512, .top = 512.0f/512, .right = 343.0f/512, .bottom = 464.0f/512, }, },
	[imgId_loading] = { .width = 8.0f, .height = 8.0f, .texcoord = { .left = 295.0f/512, .top = 463.0f/512, .right = 303.0f/512, .bottom = 455.0f/512, }, },
	[imgId_logo] = { .width = 271.0f, .height = 113.0f, .texcoord = { .left = 0.0f/512, .top = 448.0f/512, .right = 271.0f/512, .bottom = 335.0f/512, }, },
	[imgId_logo2] = { .width = 271.0f, .height = 113.0f, .texcoord = { .left = 0.0f/512, .top = 334.0f/512, .right = 271.0f/512, .bottom = 221.0f/512, }, },
	[imgId_scrollbarKnob] = { .width = 7.0f, .height = 57.0f, .texcoord = { .left = 344.0f/512, .top = 474.0f/512, .right = 351.0f/512, .bottom = 417.0f/512, }, },
	[imgId_scrollbarTrack] = { .width = 7.0f, .height = 200.0f, .texcoord = { .left = 332.0f/512, .top = 463.0f/512, .right = 339.0f/512, .bottom = 263.0f/512, }, },
	[imgId_settings] = { .width = 38.0f, .height = 33.0f, .texcoord = { .left = 272.0f/512, .top = 448.0f/512, .right = 310.0f/512, .bottom = 415.0f/512, }, },
	[imgId_wifi0] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 372.0f/512, .top = 512.0f/512, .right = 392.0f/512, .bottom = 494.0f/512, }, },
	[imgId_wifi1] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 0.0f/512, .top = 138.0f/512, .right = 20.0f/512, .bottom = 120.0f/512, }, },
	[imgId_wifi2] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 33.0f/512, .top = 171.0f/512, .right = 53.0f/512, .bottom = 153.0f/512, }, },
	[imgId_wifi3] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 311.0f/512, .top = 463.0f/512, .right = 331.0f/512, .bottom = 445.0f/512, }, },
	[imgId_wifiNull] = { .width = 20.0f, .height = 18.0f, .texcoord = { .left = 311.0f/512, .top = 444.0f/512, .right = 331.0f/512, .bottom = 426.0f/512, }, },
};
