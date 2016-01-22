#pragma once
#include "common.h"
#include "imagemap.h"

#define DRAWING_MAX_VERTICES 8192

typedef enum
{
	DRAW_MODE_INVALID = 0,
	DRAW_MODE_DRAWING,
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

void drawingEnableDepth(bool enable);
void drawingSetMode(DrawingMode mode);
void drawingSetZ(float z);
void drawingSetTex(C3D_Tex* tex);

void drawingWithColor(u32 color);
void drawingWithTex(C3D_Tex* tex, u32 color);

void drawingAddVertex(float vx, float vy, float tx, float ty);
void drawingSubmitPrim(GPU_Primitive_t prim, int vertices);

void drawingDrawQuad(float x, float y, float w, float h);
void drawingDrawImage(ImageId id, u32 color, float x, float y);
