#pragma once
#include "common.h"
#include "images.h"

typedef int ImageId;

typedef struct
{
	float width, height;
} imageInfo_s;

extern imageInfo_s* g_imageData;

#define DRAWING_MAX_VERTICES 8192

typedef enum
{
	DRAW_MODE_INVALID = 0,
	DRAW_MODE_DRAWING,
	DRAW_MODE_WAVE,
} DrawingMode;

typedef struct
{
	float position[3];
	float texcoord[2];
} drawVertex_s;

void drawingInit(void);
void drawingExit(void);
void drawingFrame(void);
void drawingSetFade(float fade);

static inline u32 drawingGetFrames(void)
{
	extern u32 g_drawFrames;
	return g_drawFrames;
}

void drawingEnableDepth(bool enable);
void drawingSetMode(DrawingMode mode);
void drawingSetZ(float z);
void drawingSetTex(C3D_Tex* tex);
void drawingSetGradient(unsigned which, float r, float g, float b, float a);

void drawingWithVertexColor(void);
void drawingWithColor(u32 color);
void drawingWithTex(C3D_Tex* tex, u32 color);

void drawingAddVertex(float vx, float vy, float tx, float ty);
void drawingSubmitPrim(GPU_Primitive_t prim, int vertices);

void drawingDrawQuad(float x, float y, float w, float h);
void drawingDrawImage(ImageId id, u32 color, float x, float y);
void drawingDrawWave(float* points, u32 num_points, float x, float width, float dy_top, float dy_bot);
