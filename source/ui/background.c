#include "background.h"

static u32 wifiStatus;
static u8 batteryLevel = 5;
static u8 charging;
static char timeString[9];
static char versionString[64];
#ifdef DBGSTRING
static char dbgString[64];
#endif

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

#define WAVE_HEIGHT 48.0f
#define WAVE_NUMPOINTS 32
#define WAVE_NUMCURVES 5

static bubble_t bubbles[BUBBLE_COUNT];
static float logoPosX, logoPosY;
static ImageId logoImg = images_logo_idx;

static inline float floatFract(float x)
{
	return x - floorf(x);
}

static float randomFloat(void)
{
	// Wichmann-Hill
	static u16 s1 = 100, s2 = 100, s3 = 100;

	s1 = (171 * s1) % 30269;
	s2 = (172 * s2) % 30307;
	s3 = (170 * s3) % 30323;

	return floatFract(s1/30269.0f + s2/30307.0f + s3/30323.0f);
}

static float randomFloatInterval(float vmin, float vmax)
{
	return randomFloat()*(vmax-vmin) + vmin;
}

static struct
{
	float phase;
	float amplitude;
	float velocity;
} waveCurves[WAVE_NUMCURVES];

//static float wavePos;
static float wavePoints[WAVE_NUMPOINTS];
static float waveDisturbance[WAVE_NUMPOINTS];
static u32 waveDisturbancePos;
static float waveDisturbanceCur;

static const ImageId batteryLevels[] =
{
	images_battery0_idx,
	images_battery0_idx,
	images_battery1_idx,
	images_battery2_idx,
	images_battery3_idx,
	images_battery4_idx,
};

static const ImageId wifiLevels[] =
{
	images_wifiNull_idx,
	images_wifi1_idx,
	images_wifi2_idx,
	images_wifi3_idx,
};

static float randf()
{
	return (float)rand()/(float)RAND_MAX;
}

void backgroundInit(void)
{
	int i = 0;
	for (i = 0; i < BUBBLE_COUNT; i ++)
	{
		bubbles[i].x = rand() % 400;
		bubbles[i].y = 172 + (rand() % 308);
		bubbles[i].z = randf();
		bubbles[i].angle = randf();
		bubbles[i].angv = 0.02f*randf();
		bubbles[i].fade = 15;
	}
	float height = WAVE_HEIGHT*0.8f;
	for (i = 0; i < WAVE_NUMCURVES; i ++)
	{
		waveCurves[i].amplitude = randomFloatInterval(0, height);
		waveCurves[i].phase = randomFloat();
		waveCurves[i].velocity = randomFloatInterval(1.0f, 4.0f);
		height -= waveCurves[i].amplitude;
	}
	waveCurves[WAVE_NUMCURVES-1].velocity = M_TAU;
	sprintf(versionString, "%s \xEE\x80\x9D %s", launchGetLoader()->name, VERSION);
}

void waveDisturb(float amount)
{
	waveDisturbanceCur += amount;
	if (waveDisturbanceCur > WAVE_HEIGHT)
		waveDisturbanceCur = WAVE_HEIGHT;
}

void waveUpdate(void)
{
	waveDisturbance[waveDisturbancePos] = waveDisturbanceCur;

	for (u32 i = 0; i < WAVE_NUMPOINTS; i ++)
	{
		float x = (float)i / (WAVE_NUMPOINTS-1);
		float y = 120.0f;
		float dist = waveDisturbance[(waveDisturbancePos + i + 1) % WAVE_NUMPOINTS];
		for (u32 j = 0; j < WAVE_NUMCURVES; j ++)
		{
			float ampl = waveCurves[j].amplitude;
			if (j == (WAVE_NUMCURVES-1))
				ampl = dist;
			y += 0.5f*ampl * sinf(M_TAU*(waveCurves[j].velocity*x + waveCurves[j].phase));
		}
		wavePoints[i] = y;
	}

	for (u32 j = 0; j < WAVE_NUMCURVES; j ++)
	{
		float variance = randomFloatInterval(0.95f, 1.05f);
		if (j == (WAVE_NUMCURVES-1))
			variance *= waveCurves[j].velocity;
		waveCurves[j].phase += variance/60.0f;
	}

	waveDisturbancePos = (waveDisturbancePos + 1) % WAVE_NUMPOINTS;
	waveDisturbanceCur -= randomFloatInterval(0.95f, 1.05f);
	if (waveDisturbanceCur < 0.0f)
		waveDisturbanceCur = 0.0f;
}

static void bubbleUpdate(bubble_t* bubble)
{
	// Float up the screen.
	bubble->y -= 2;

	// Check if faded away, then reset if gone.
	if (bubble->fade < 10)
	{
		bubble->x = rand() % 400;
		bubble->y = 470 + (rand() % 10);
		bubble->z = randf();
		bubble->angle = randf();
		bubble->angv = 0.02f*randf();
		bubble->fade = 15;
	}
	// Check if too far up screen and start fizzling away.
	else if (bubble->y < 172)
		bubble->fade -= 10;
	// Otherwise make sure the bubble is visible.
	else if (bubble->fade < 255)
		bubble->fade += 10;

	bubble->angle += bubble->angv;
	bubble->angleSin = sinf(M_TAU*bubble->angle);
}

static bool checkLogoAdv(u32 down)
{
	static const u32 params[] = { KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_SELECT };
	static u32 state, timeout;

	if (down & params[state])
	{
		state++;
		timeout = 30;
		if (state == sizeof(params)/sizeof(params[0]))
		{
			state = 0;
			return true;
		}
	}

	if (timeout && !--timeout)
		state = 0;

	return false;
}

void backgroundUpdate(void)
{
	u32 i, j;
	u32 frames = drawingGetFrames();
	u32 kDown = hidKeysDown();

	wifiStatus = osGetWifiStrength();
	PTMU_GetBatteryLevel(&batteryLevel);
	PTMU_GetBatteryChargeState(&charging);

	u64 timeInSeconds = osGetTime() / 1000;
	u64 dayTime = timeInSeconds % SECONDS_IN_DAY;
	u8 hour = dayTime / SECONDS_IN_HOUR;
	u8 min = (dayTime % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
	u8 seconds = dayTime % SECONDS_IN_MINUTE;
	sprintf(timeString, "%02d:%02d:%02d", hour, min, seconds);
#ifdef DBGSTRING
	sprintf(dbgString, "fs:%lu gpu: %.2f%% cpu: %.2f%%", frames, C3D_GetDrawingTime()*6, C3D_GetProcessingTime()*6);
#endif

	if (kDown & (KEY_LEFT|KEY_RIGHT))
		waveDisturb(WAVE_HEIGHT);
	else if (kDown & (KEY_UP|KEY_DOWN))
		waveDisturb(WAVE_HEIGHT*0.5f);

	// Update graphical effects
	for (j = frames; j; j --)
	{
		waveUpdate();
		for (i = 0; i < BUBBLE_COUNT; i ++)
			bubbleUpdate(&bubbles[i]);
	}

	// Update logo
	if (logoImg == images_logo2_idx)
		logoPosX += frames/64.0f;
	logoPosY -= frames/192.0f;
	if (checkLogoAdv(kDown))
		logoImg = images_logo2_idx;
}

void bubbleDraw(bubble_t* bubble, float top, float iod)
{
	if ((bubble->y+32) <= top)
		return; // Nothing to do
	u32 color = ((u32)bubble->fade << 24) | 0xFFFFFF;
	float x = bubble->x + iod*(10+10*bubble->z) + 16*bubble->angleSin;
	float y = bubble->y - top;
	if (top > 0.0f)
		x -= (400-320)/2;
	drawingDrawImage(images_bubble_idx, color, x, y);
}

void backgroundDrawTop(float iod)
{
	int i;

	drawingSetMode(DRAW_MODE_DRAWING);

	// Clear screen
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(1.0f);
	drawingEnableDepth(false);
	drawingWithColor(0xFFFF8400);
	drawingDrawQuad(0.0f, 0.0f, 400.0f, 240.0f);

	// Draw the wave
	drawingSetMode(DRAW_MODE_WAVE);
	drawingWithVertexColor();
	drawingSetGradient(0, 1.0f, 1.0f, 1.0f, 0.0f);
	drawingSetGradient(1, 1.0f, 1.0f, 1.0f, 1.0f);
	drawingDrawWave(wavePoints, WAVE_NUMPOINTS, 0.0f, 400.0f, -4.0f, +0.0f);
	drawingSetGradient(0, 1.0f, 1.0f, 1.0f, 1.0f);
	drawingSetGradient(1, 0.2588f, 0.6392f, 1.0f, 1.0f);
	drawingDrawWave(wavePoints, WAVE_NUMPOINTS, 0.0f, 400.0f, +0.0f, +8.0f);
	drawingSetGradient(0, 0.2588f, 0.6392f, 1.0f, 1.0f);
	drawingDrawWave(wavePoints, WAVE_NUMPOINTS, 0.0f, 400.0f, +8.0f, +240.0f);
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingEnableDepth(true);

	// Draw bubbles!
	drawingSetZ(0.6f);
	for (i = 0; i < BUBBLE_COUNT; i ++)
		bubbleDraw(&bubbles[i], 0.0f, iod);

	// Draw HUD
	drawingSetZ(0.5f);

	float logo_width = g_imageData[logoImg].width;
	float logo_left = floorf(0.5f*(400.0f - logo_width));
	float logo_right = 400.0f - logo_left;

	textSetColor(0xFFFFFFFF);
	textDrawInBox(timeString, 0, 0.5f, 0.5f, 15.0f, 0.0f, 400.0f);
	textDrawInBox(versionString, 1, 0.5f, 0.5f, 200.0f, 80.0f, logo_right);
#ifdef DBGSTRING
	textDrawInBox(dbgString, -1, 0.5f, 0.5f, 214.0f, 20.0f, logo_right);
#endif

	float posX = 20.0f*sinf(C3D_Angle(logoPosX));
	float posY =  6.0f*sinf(C3D_Angle(logoPosY));
	drawingDrawImage(logoImg, 0xFFFFFFFF, logo_left+posX-iod*6.0f, 63.0f+posY);
	drawingDrawImage(wifiLevels[wifiStatus], 0xFFFFFFFF, 0.0f, 0.0f);
	drawingDrawImage(charging ? images_batteryCharge_idx : batteryLevels[batteryLevel], 0xFFFFFFFF, 400.0f-27, 0.0f);
}

void backgroundDrawBot(void)
{
	int i;

	drawingSetMode(DRAW_MODE_DRAWING);

	// Clear screen
	drawingSetZ(0.8f);
	drawingEnableDepth(false);
	drawingWithColor(0xFFFFA342);
	drawingDrawQuad(0.0f, 0.0f, 320.0f, 240.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);
	drawingEnableDepth(true);

	// Draw bubbles!
	drawingSetZ(0.6f);
	for (i = 0; i < BUBBLE_COUNT; i ++)
		bubbleDraw(&bubbles[i], 240.0f, 0.0f);
}
