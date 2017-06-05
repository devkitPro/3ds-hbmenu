#include "text.h"

static C3D_Tex* s_glyphSheets;
static int s_textLang = CFG_LANGUAGE_EN;

void textInit(void)
{
	// Ensure the shared system font is mapped
	fontEnsureMapped();

	// Load the glyph texture sheets
	int i;
	TGLP_s* glyphInfo = fontGetGlyphInfo();
	s_glyphSheets = malloc(sizeof(C3D_Tex)*glyphInfo->nSheets);
	for (i = 0; i < glyphInfo->nSheets; i ++)
	{
		C3D_Tex* tex = &s_glyphSheets[i];
		tex->data = fontGetGlyphSheetTex(i);
		tex->fmt = glyphInfo->sheetFmt;
		tex->size = glyphInfo->sheetSize;
		tex->width = glyphInfo->sheetWidth;
		tex->height = glyphInfo->sheetHeight;
		tex->param = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)
			| GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_EDGE) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_EDGE);
		tex->border = 0;
		tex->lodParam = 0;
	}

	FILE* f = fopen("sdmc:/3ds/locale.bin", "rb");
	if (f) {
		u8 c;
		if (fread(&c, sizeof(c), 1, f) == 1) {
			// Support normal text editor
			if (c >= '0' && c <= '9') {
				c -= '0';
			} else if (c >= 'a' && c <= 'z') {
				c = c - 'a' + 0xa;
			} else if (c >= 'A' && c <= 'Z') {
				c = c - 'A' + 0xa;
			}

			s_textLang = c;
		}
		fclose(f);

		// If the language id is invalid, fall back to system locale.
		if (c >= 0 && c <= 11) {
			return ;
		}
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
	C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
	C3D_TexEnvColor(env, color);
}

static inline float maxf(float a, float b)
{
	return a > b ? a : b;
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
			int glyphIdx = fontGlyphIndexFromCodePoint(code);
			charWidthInfo_s* cwi = fontGetCharWidthInfo(glyphIdx);
			width += cwi->charWidth;
		}
	} while (code > 0);
	return maxf(width, maxWidth);
}

void textDraw(float x, float y, float scaleX, float scaleY, bool baseline, const char* text)
{
	ssize_t  units;
	uint32_t code;

	const uint8_t* p = (const uint8_t*)text;
	float firstX = x;
	u32 flags = GLYPH_POS_CALC_VTXCOORD | (baseline ? GLYPH_POS_AT_BASELINE : 0);
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
			y += ceilf(scaleY*fontGetInfo()->lineFeed);
		}
		else if (code > 0)
		{
			int glyphIdx = fontGlyphIndexFromCodePoint(code);
			fontGlyphPos_s data;
			fontCalcGlyphPos(&data, glyphIdx, flags, scaleX, scaleY);

			// Draw the glyph
			drawingSetTex(&s_glyphSheets[data.sheetIndex]);
			drawingAddVertex(x+data.vtxcoord.left,  y+data.vtxcoord.bottom, data.texcoord.left,  data.texcoord.bottom);
			drawingAddVertex(x+data.vtxcoord.right, y+data.vtxcoord.bottom, data.texcoord.right, data.texcoord.bottom);
			drawingAddVertex(x+data.vtxcoord.left,  y+data.vtxcoord.top,    data.texcoord.left,  data.texcoord.top);
			drawingAddVertex(x+data.vtxcoord.right, y+data.vtxcoord.top,    data.texcoord.right, data.texcoord.top);
			drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

			x += data.xAdvance;

		}
	} while (code > 0);
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
