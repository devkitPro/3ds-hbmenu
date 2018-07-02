#include "drawing.h"
#include "program_shbin.h"

// Global variables
imageInfo_s* g_imageData;
shaderProgram_s g_drawProg;
u8 uLoc_projection;
C3D_AttrInfo g_drawAttrInfo;
C3D_BufInfo g_drawBufInfo;
u32 g_drawFrames = 1;

// Static variables
static DVLB_s* s_programBin;
static C3D_RenderTarget* s_targets[3];
static C3D_Mtx s_projectionTop, s_projectionBot;
static DrawingMode s_drawingMode;
static drawVertex_s* s_drawBuffer;
static int s_drawBufferPos;
static float s_drawBufferZ = 0.5;
static C3D_Tex* s_curTex;
static C3D_Tex s_imagesTex;
static Tex3DS_Texture s_imagesTexInfo;
static float s_screenWidth;
static float s_brightnessLevel;
static float s_brightnessFade = 2.5f / 60;
static bool s_drew;
static u32 s_lastFrame;

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

void lzssDecompress(const void *in, void *out, u32 size);

static void loadImages(void)
{
	FILE* f = fopen("romfs:/gfx/images.t3x", "rb");
	if (!f) svcBreak(USERBREAK_PANIC);
	s_imagesTexInfo = Tex3DS_TextureImportStdio(f, &s_imagesTex, NULL, false);
	fclose(f);
	if (!s_imagesTexInfo) svcBreak(USERBREAK_PANIC);

	size_t numSubTex = Tex3DS_GetNumSubTextures(s_imagesTexInfo);
	g_imageData = (imageInfo_s*)malloc(numSubTex*sizeof(imageInfo_s));
	if (!g_imageData) svcBreak(USERBREAK_PANIC);

	for (size_t i = 0; i < numSubTex; i ++)
	{
		const Tex3DS_SubTexture* subtex = Tex3DS_GetSubTexture(s_imagesTexInfo, i);
		g_imageData[i].width = subtex->width;
		g_imageData[i].height = subtex->height;
	}
}

void drawingInit(void)
{
	gfxInitDefault();
	gfxSet3D(true);
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	// Load programs
	s_programBin = DVLB_ParseFile((u32*)program_shbin, program_shbin_size);
	shaderProgramInit(&g_drawProg);
	shaderProgramSetVsh(&g_drawProg, &s_programBin->DVLE[0]);

	// Load uniform positions
	uLoc_projection = shaderInstanceGetUniformLocation(g_drawProg.vertexShader, "projection");

	// Create vertex buffer
	s_drawBuffer = (drawVertex_s*)linearAlloc(sizeof(drawVertex_s)*DRAWING_MAX_VERTICES);

	// Configure attribute informations
	AttrInfo_Init(&g_drawAttrInfo);
	AttrInfo_AddLoader(&g_drawAttrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(&g_drawAttrInfo, 1, GPU_FLOAT, 2); // v1=texcoord

	// Configure buffer informations
	BufInfo_Init(&g_drawBufInfo);
	BufInfo_Add(&g_drawBufInfo, s_drawBuffer, sizeof(drawVertex_s), 2, 0x10);

	// Create rendering targets
	s_targets[0] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	s_targets[1] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	s_targets[2] = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetOutput(s_targets[0], GFX_TOP,    GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetSetOutput(s_targets[1], GFX_TOP,    GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetSetOutput(s_targets[2], GFX_BOTTOM, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);

	// Precalc stuff
	Mtx_OrthoTilt(&s_projectionTop, 0.0, 400.0, 240.0, 0.0, 0.0, 1.0, true);
	Mtx_OrthoTilt(&s_projectionBot, 0.0, 320.0, 240.0, 0.0, 0.0, 1.0, true);
	loadImages();
}

void drawingExit(void)
{
	// Free the images
	free(g_imageData);
	C3D_TexDelete(&s_imagesTex);
	Tex3DS_TextureFree(s_imagesTexInfo);

	// Free the shader programs
	shaderProgramFree(&g_drawProg);
	DVLB_Free(s_programBin);

	// Deinitialize graphics
	C3D_Fini();
	gfxExit();
}

void drawingSetFade(float fade)
{
	s_brightnessFade = fade;
	s_brightnessLevel = fade > 0.0f ? 0.0f : 1.0f;
}

void drawingEnableDepth(bool enable)
{
	// Configure depth test to overwrite pixels with the same depth (needed to draw overlapping graphics)
	C3D_DepthTest(enable, enable ? GPU_GEQUAL : GPU_ALWAYS, GPU_WRITE_ALL);
}

void drawingSetMode(DrawingMode mode)
{
	if (mode == s_drawingMode) return;
	s_drawingMode = mode;
	switch (mode)
	{
		case DRAW_MODE_DRAWING:
			C3D_BindProgram(&g_drawProg);
			C3D_SetAttrInfo(&g_drawAttrInfo);
			C3D_SetBufInfo(&g_drawBufInfo);
			break;
		default:
			break;
	}
}

void drawingSetZ(float z)
{
	s_drawBufferZ = z;
}

void drawingSetTex(C3D_Tex* tex)
{
	if (tex == s_curTex) return;
	s_curTex = tex;
	C3D_TexBind(0, tex);
}

void drawingWithColor(u32 color)
{
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_Both, GPU_CONSTANT, 0, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
	C3D_TexEnvColor(env, color);
}

void drawingWithTex(C3D_Tex* tex, u32 color)
{
	drawingSetTex(tex);
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_CONSTANT, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
	C3D_TexEnvColor(env, color);
}

void drawingAddVertex(float vx, float vy, float tx, float ty)
{
	drawVertex_s* vtx = &s_drawBuffer[s_drawBufferPos++];
	vtx->position[0] = vx;
	vtx->position[1] = vy;
	vtx->position[2] = s_drawBufferZ;
	vtx->texcoord[0] = tx;
	vtx->texcoord[1] = ty;
}

void drawingSubmitPrim(GPU_Primitive_t prim, int vertices)
{
	C3D_DrawArrays(prim, s_drawBufferPos-vertices, vertices);
}

static void drawingFade(float width, float height)
{
	if (s_brightnessLevel >= 1.0f) return;
	drawingWithColor(((u32)((1-s_brightnessLevel)*255)) << 24);
	drawingDrawQuad(0, 0, width, height);
}

static void drawingTopScreen(float iod)
{
	const uiStateInfo_s* ui;

	// Set projection matrix
	if (iod <= 0.0f)
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &s_projectionTop);

	// Draw background
	ui = g_uiStateBg;
	if (ui->drawTop) ui->drawTop(iod);

	// Draw current state
	ui = uiGetStateInfo();
	if (ui->drawTop) ui->drawTop(iod);

	/*
	// Draw debug text
	static char debugText[128];
	snprintf(debugText, sizeof(debugText), "Cmdbuf usage: %d%%", (int)(C3D_GetCmdBufUsage()*100));
	*/

	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.0f);
	textSetColor(0xFFFFFFFF);
	//textDraw(8.0f, 32.0f, 0.5f, 0.5f, true, debugText);

	// Draw fade
	drawingFade(400, 240);
}

static void drawingBottomScreen(void)
{
	const uiStateInfo_s* ui;

	// Set projection matrix
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &s_projectionBot);

	// Draw background
	ui = g_uiStateBg;
	if (ui->drawBot) ui->drawBot();

	// Draw current state
	ui = uiGetStateInfo();
	if (ui->drawBot) ui->drawBot();

	if (uiIsBusy())
	{
		// Draw 'busy' icon
		int i;
		static float counter = 0;

		drawingSetMode(DRAW_MODE_DRAWING);
		drawingSetZ(0.0f);

		drawingWithColor(0x80FFFFFF);
		drawingDrawQuad(0.0f, 80.0f, 320.0f, 100.0f);

		for (i = 0; i < 8; i ++)
		{
			float angle = ((float)i/8.0f + counter)*M_TAU;
			float x = 320.0f/2 + 24*cosf(angle);
			float y = 240.0f/2 + 24*sinf(angle);
			drawingDrawImage(images_loading_idx, 0xFFFFFFFF, x-4, y-4);
		}
		counter += g_drawFrames*0.5f/60;

		textSetColor(0xFF000000); // black
		textDrawInBox(textGetString(StrId_Loading), 0, 0.5f, 0.5f, 170.f, 0.0f, 320.0f);
	}

	// Draw fade
	drawingFade(320, 240);
}

void drawingFrame(void)
{
	float slider = osGet3DSliderState();
	float iod = slider/3;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	u32 frame = C3D_FrameCounter(0);
	if (s_drew)
		g_drawFrames = frame-s_lastFrame;
	else
		s_drew = true;
	s_lastFrame = frame;
	s_drawBufferPos = 0;

	// Update brightness level
	s_brightnessLevel += s_brightnessFade*g_drawFrames;
	if (s_brightnessLevel < 0.0f)
	{
		s_brightnessLevel = 0.0f;
		s_brightnessFade = 0.0f;
	} else if (s_brightnessLevel >= 1.0f)
	{
		s_brightnessLevel = 1.0f;
		s_brightnessFade = 0.0f;
	}

	// Top screen
	s_screenWidth = 400.0f;
	C3D_FrameDrawOn(s_targets[0]);
	drawingTopScreen(-iod);
	if (iod > 0.0f)
	{
		C3D_FrameDrawOn(s_targets[1]);
		drawingTopScreen(iod);
	}

	// Bottom screen
	s_screenWidth = 320.0f;
	C3D_FrameDrawOn(s_targets[2]);
	drawingBottomScreen();

	C3D_FrameEnd(0);
}

void drawingDrawQuad(float x, float y, float w, float h)
{
	drawingAddVertex(x,   y+h, 0.0f,  0.0f);
	drawingAddVertex(x+w, y+h, 0.75f, 0.0f);
	drawingAddVertex(x,   y,   0.0f,  0.75f);
	drawingAddVertex(x+w, y,   0.75f, 0.75f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);
}

void drawingDrawImage(ImageId id, u32 color, float x, float y)
{
	const imageInfo_s* p = &g_imageData[id];
	drawingWithTex(&s_imagesTex, color);

	// Calculate texcoords
	float tcTopLeft[2], tcTopRight[2], tcBotLeft[2], tcBotRight[2];
	const Tex3DS_SubTexture* subtex = Tex3DS_GetSubTexture(s_imagesTexInfo, id);
	Tex3DS_SubTextureBottomLeft (subtex, &tcBotLeft[0],  &tcBotLeft[1]);
	Tex3DS_SubTextureBottomRight(subtex, &tcBotRight[0], &tcBotRight[1]);
	Tex3DS_SubTextureTopLeft    (subtex, &tcTopLeft[0],  &tcTopLeft[1]);
	Tex3DS_SubTextureTopRight   (subtex, &tcTopRight[0], &tcTopRight[1]);

	drawingAddVertex(x,          y+p->height, tcBotLeft[0], tcBotLeft[1]);
	drawingAddVertex(x+p->width, y+p->height, tcBotRight[0], tcBotRight[1]);
	drawingAddVertex(x,          y,           tcTopLeft[0], tcTopLeft[1]);
	drawingAddVertex(x+p->width, y,           tcTopRight[0], tcTopRight[1]);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);
}
