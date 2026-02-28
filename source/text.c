#include "text.h"

#define NUM_ASCII_CHARS 128
#define SHEETS_PER_BIG_SHEET 32

static C3D_Tex* s_glyphSheets;
static float s_textScale;
static int s_textLang = CFG_LANGUAGE_EN;
static uint32_t s_numSheetsThatWereCombined;
static charWidthInfo_s* s_asciiCacheCharWidth[NUM_ASCII_CHARS];
// @Note: Could use s_asciiCacheCharWidth to reimplement fontCalcGlyphPos, but it would cache slightly less computations.
static fontGlyphPos_s s_asciiCacheGlyphPos[NUM_ASCII_CHARS];

static fontGlyphPos_s _textGetGlyphPosFromCodePoint(uint32_t code, uint32_t flags, float scaleX, float scaleY)
{
	fontGlyphPos_s result;
	int glyphIdx = fontGlyphIndexFromCodePoint(NULL, code);
	fontCalcGlyphPos(&result, NULL, glyphIdx, flags, scaleX, scaleY);

	if (result.sheetIndex < s_numSheetsThatWereCombined)
	{
		uint32_t indexWithinBigSheet = result.sheetIndex % SHEETS_PER_BIG_SHEET;
		result.sheetIndex /= SHEETS_PER_BIG_SHEET;

		// Readjust glyph UVs to account for being a part of the combined texture.
		result.texcoord.top    = (result.texcoord.top    + (SHEETS_PER_BIG_SHEET - indexWithinBigSheet - 1)) / (float) SHEETS_PER_BIG_SHEET;
		result.texcoord.bottom = (result.texcoord.bottom + (SHEETS_PER_BIG_SHEET - indexWithinBigSheet - 1)) / (float) SHEETS_PER_BIG_SHEET;
	}
	else
	{
		result.sheetIndex -= s_numSheetsThatWereCombined * SHEETS_PER_BIG_SHEET;
	}

	return result;
}

static void fillSheet(C3D_Tex *tex, void *data, TGLP_s *glyphInfo)
{
	tex->data     = data;
	tex->fmt      = glyphInfo->sheetFmt;
	tex->size     = glyphInfo->sheetSize;
	tex->width    = glyphInfo->sheetWidth;
	tex->height   = glyphInfo->sheetHeight;
	tex->param    = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)
		| GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_EDGE) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_EDGE);
	tex->border   = 0;
	tex->lodParam = 0;
}

void textInit(void)
{
	// Ensure the shared system font is mapped
	fontEnsureMapped();

	CFNT_s* font = fontGetSystemFont();
	// Load the glyph texture sheets
	TGLP_s* glyphInfo = fontGetGlyphInfo(NULL);

	// As it turns out, the sytem font texture sheets are all 128x32 pixels and adjacent in memory! We can reinterpet
	// the memory starting at sheet 0 and describe a much bigger texture that encompasses all of the ASCII glyphs and
	// make our cache use that instead of the individual sheets. This will massively improve performance by reducing
	// texture swaps within a piece of text, down to 0 if it's all English. We don't need any extra linear allocating to
	// do this!
	uint32_t numSheetsBig   = glyphInfo->nSheets / SHEETS_PER_BIG_SHEET;
	uint32_t numSheetsSmall = glyphInfo->nSheets % SHEETS_PER_BIG_SHEET;
	uint32_t numSheetsTotal = numSheetsBig + numSheetsSmall;
	s_numSheetsThatWereCombined = glyphInfo->nSheets - numSheetsSmall;

	s_glyphSheets = malloc(sizeof(C3D_Tex)*numSheetsTotal);
	s_textScale = 30.0f / glyphInfo->cellHeight;
	for (uint32_t i = 0; i < numSheetsBig; i++)
	{
		C3D_Tex* tex = &s_glyphSheets[i];
		fillSheet(tex, fontGetGlyphSheetTex(font, i * SHEETS_PER_BIG_SHEET), glyphInfo);
		tex->height = (uint16_t) (tex->height * SHEETS_PER_BIG_SHEET);
		tex->size   = tex->size * SHEETS_PER_BIG_SHEET;
	}

	for (uint32_t i = 0; i < numSheetsSmall; i++)
	{
		fillSheet(&s_glyphSheets[numSheetsBig + i], fontGetGlyphSheetTex(font, numSheetsBig * SHEETS_PER_BIG_SHEET + i), glyphInfo);
	}

	// Cache up front the results of fontGetCharWidthInfo and fontCalcGlyphPos for ASCII characters, since these functions
	// can potentially take a long time.
	for (uint32_t i = 0; i < NUM_ASCII_CHARS; i++)
	{
		int glyphIdx = fontGlyphIndexFromCodePoint(NULL, i);
		s_asciiCacheCharWidth[i] = fontGetCharWidthInfo(NULL, glyphIdx);
		s_asciiCacheGlyphPos[i]  = _textGetGlyphPosFromCodePoint(i, GLYPH_POS_CALC_VTXCOORD, 1, 1);
	}

	Result res = cfguInit();
	if (R_SUCCEEDED(res))
	{
		u8 lang;
		res = CFGU_GetSystemLanguage(&lang);
		if (R_SUCCEEDED(res))
			s_textLang = lang;
		cfguExit();
	}
}

void textExit(void)
{
	free(s_glyphSheets);
}

int textGetLang(void)
{
	return s_textLang;
}

const char* textGetString(StrId id)
{
	const char* str = g_strings[id][s_textLang];
	if (!str) str = g_strings[id][CFG_LANGUAGE_EN];
	return str;
}

void textSetColor(u32 color)
{
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, 0, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_CONSTANT, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
	C3D_TexEnvColor(env, color);
}

static inline float maxf(float a, float b)
{
	return a > b ? a : b;
}

charWidthInfo_s *textGetCharWidthFromCodePoint(uint32_t code)
{
	charWidthInfo_s *result;

	if (code < NUM_ASCII_CHARS)
	{
		result = s_asciiCacheCharWidth[code];
	}
	else
	{
		int glyphIdx = fontGlyphIndexFromCodePoint(NULL, code);
		result = fontGetCharWidthInfo(NULL, glyphIdx);
	}

	return result;
}

fontGlyphPos_s textGetGlyphPosFromCodePoint(uint32_t code, uint32_t flags, float scaleX, float scaleY)
{
	if (code < NUM_ASCII_CHARS)
	{
		fontGlyphPos_s result = s_asciiCacheGlyphPos[code];

		if ((flags & GLYPH_POS_AT_BASELINE))
		{
			float baselineOffset    = fontGetSystemFont()->finf.tglp->baselinePos;
			result.vtxcoord.top    -= baselineOffset;
			result.vtxcoord.bottom -= baselineOffset;
		}

		result.xOffset         *= scaleX;
		result.xAdvance        *= scaleX;
		result.width           *= scaleX;
		result.vtxcoord.left   *= scaleX;
		result.vtxcoord.right  *= scaleX;
		result.vtxcoord.top    *= scaleY;
		result.vtxcoord.bottom *= scaleY;
		return result;
	}
	else
	{
		return _textGetGlyphPosFromCodePoint(code, flags, scaleX, scaleY);
	}
}

float textCalcWidth(const char* text)
{
	float    width = 0.0f;
	float    maxWidth = 0.0f;
	ssize_t  units;
	uint32_t code;
	const uint8_t* p = (const uint8_t*)text;
	do
	{
		if (!*p) break;
		units = decode_utf8(&code, p);
		if (units == -1)
			break;
		p += units;

		if (code == '\n')
		{
			maxWidth = maxf(width, maxWidth);
			width = 0.0f;
			continue;
		}

		if (code > 0)
		{
			charWidthInfo_s* cwi = textGetCharWidthFromCodePoint(code);
			width += cwi->charWidth;
		}
	} while (code > 0);
	return s_textScale*maxf(width, maxWidth);
}

typedef struct SortedGlyph
{
	int indexBuf;
	int indexSheet;
	float x, y;
} SortedGlyph;

static int cmpGlyphSort(const void *p1, const void *p2)
{
	const SortedGlyph lhs = *(SortedGlyph*)p1;
	const SortedGlyph rhs = *(SortedGlyph*)p2;

	return lhs.indexSheet - rhs.indexSheet;
}

#define MAX_GLYPHS_PER_STRING 256
static fontGlyphPos_s s_bufGlyph[MAX_GLYPHS_PER_STRING];
static SortedGlyph    s_bufSort[MAX_GLYPHS_PER_STRING];
static int 			      s_glyphCount;

void textDraw(float x, float y, float scaleX, float scaleY, bool baseline, const char* text)
{
	ssize_t  units;
	uint32_t code;

	const uint8_t* p = (const uint8_t*)text;
	float firstX = x;
	u32 flags = GLYPH_POS_CALC_VTXCOORD | (baseline ? GLYPH_POS_AT_BASELINE : 0);
	scaleX *= s_textScale;
	scaleY *= s_textScale;

	int vertexCount = 0;
	C3D_Tex *sheetCur = NULL;

	s_glyphCount = 0;

	do
	{
		if (!*p) break;
		units = decode_utf8(&code, p);
		if (units == -1)
			break;
		p += units;
		if (code == '\n')
		{
			x = firstX;
			y += ceilf(scaleY*fontGetInfo(NULL)->lineFeed);
		}
		else if (code > 0)
		{
			s_bufGlyph[s_glyphCount] = textGetGlyphPosFromCodePoint(code, flags, scaleX, scaleY);
			s_bufSort[s_glyphCount]  = (SortedGlyph) { s_glyphCount, s_bufGlyph[s_glyphCount].sheetIndex, x, y };
			x += s_bufGlyph[s_glyphCount].xAdvance;
			s_glyphCount++;
		}
	} while (code > 0 && s_glyphCount < MAX_GLYPHS_PER_STRING);  // If the string is too long, we'll truncate it.

	// For performance reasons, we want to try to batch up as many glyphs per draw call as we can. To do this, all the
	// glyphs must be in the same texture. But, the system font is split up into many textures that contain 5 glyphs
	// each. If we were to batch glyphs naively, we'd typically swapping textures constantly within a piece of text, and
	// that's devastating for performance. The best we can do is sort by glyph texture and batch that way.
	qsort(s_bufSort, s_glyphCount, sizeof(s_bufSort[0]), cmpGlyphSort);

	for (int i = 0; i < s_glyphCount; i++)
	{
		SortedGlyph     sorted     = s_bufSort[i];
		fontGlyphPos_s *data       = &s_bufGlyph[sorted.indexBuf];
		C3D_Tex        *sheetGlyph = &s_glyphSheets[sorted.indexSheet];
		if (sheetCur != sheetGlyph)
		{
			if (vertexCount > 0)
			{
				drawingSubmitPrim(GPU_TRIANGLES, vertexCount);
				vertexCount = 0;
			}

			sheetCur = sheetGlyph;
			drawingSetTex(sheetCur);
		}

		// Draw the glyph
		drawingAddVertex(sorted.x+data->vtxcoord.left,  sorted.y+data->vtxcoord.bottom, data->texcoord.left,  data->texcoord.bottom);
		drawingAddVertex(sorted.x+data->vtxcoord.right, sorted.y+data->vtxcoord.bottom, data->texcoord.right, data->texcoord.bottom);
		drawingAddVertex(sorted.x+data->vtxcoord.left,  sorted.y+data->vtxcoord.top,    data->texcoord.left,  data->texcoord.top);
		drawingAddVertex(sorted.x+data->vtxcoord.left,  sorted.y+data->vtxcoord.top,    data->texcoord.left,  data->texcoord.top);
		drawingAddVertex(sorted.x+data->vtxcoord.right, sorted.y+data->vtxcoord.bottom, data->texcoord.right, data->texcoord.bottom);
		drawingAddVertex(sorted.x+data->vtxcoord.right, sorted.y+data->vtxcoord.top,    data->texcoord.right, data->texcoord.top);

		vertexCount += 6;
	}

	if (vertexCount > 0)
	{
		drawingSubmitPrim(GPU_TRIANGLES, vertexCount);
	}
}

void textDrawInBox(const char* text, int orientation, float scaleX, float scaleY, float baseline, float left, float right)
{
	float bwidth = right-left;
	float twidth = scaleX*textCalcWidth(text);
	if (twidth > bwidth)
	{
		scaleX *= bwidth / twidth;
		twidth = bwidth;
	}
	float x;
	if (orientation < 0)
		x = left;
	else if (orientation > 0)
		x = floorf(right-twidth);
	else
		x = left + floorf((bwidth-twidth)/2);
	textDraw(x, baseline, scaleX, scaleY, true, text);
}
