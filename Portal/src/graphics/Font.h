#pragma once

#include <stdint.h>
#include <stb_truetype.h>
#include <SDL3/SDL.h>


struct FontData
{
	uint8_t* data;
	int size;
	stbtt_fontinfo info;
};

struct Font
{
	FontData* data;
	float size;

#define FONT_CHAR_OFFSET ' '
#define MAX_FONT_CHARACTERS (255 - FONT_CHAR_OFFSET)
	stbtt_bakedchar characters[MAX_FONT_CHARACTERS];

	SDL_GPUTexture* texture;
	int width, height;
};


bool LoadFontData(FontData* fontData, const char* path);
void DestroyFontData(FontData* fontData);
void InitFont(Font* font, FontData* fontData, float size);
