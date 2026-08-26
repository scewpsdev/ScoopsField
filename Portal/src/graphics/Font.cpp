#include "Font.h"

#include "Application.h"

#include "utils/BumpAllocator.h"


bool LoadFontData(FontData* fontData, const char* path)
{
	size_t fileSize;
	void* data = SDL_LoadFile(path, &fileSize);

	if (data)
	{
		fontData->data = (uint8_t*)data;
		fontData->size = (int)fileSize;

		stbtt_InitFont(&fontData->info, (uint8_t*)data, 0);

		return true;
	}

	return false;
}

void DestroyFontData(FontData* fontData)
{
	SDL_free(fontData->data);
}

void InitFont(Font* font, FontData* fontData, float size)
{
	font->data = fontData;
	font->size = size;

	const int atlasSize = 1024;
	uint8_t* pixels = BumpAllocatorMalloc(&memory->transientAllocator, atlasSize * atlasSize);

	stbtt_BakeFontBitmap(fontData->data, 0, size, pixels, atlasSize, atlasSize, FONT_CHAR_OFFSET, MAX_FONT_CHARACTERS, font->characters);

	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	textureInfo.width = atlasSize;
	textureInfo.height = atlasSize;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	font->texture = SDL_CreateGPUTexture(device, &textureInfo);

	SetTextureData(font->texture, pixels, atlasSize * atlasSize, atlasSize, atlasSize, 1, cmdBuffer);

	font->width = atlasSize;
	font->height = atlasSize;
}
