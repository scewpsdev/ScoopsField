#pragma once

#include <SDL3/SDL.h>


struct TextureInfo
{
	SDL_GPUTextureFormat format;
	int width;
	int height;
	int depth;
	int numMips;
	int numLayers;
	int numFaces;
};

enum TextureSampler
{
	TEXTURE_SAMPLER_DEFAULT,
	TEXTURE_SAMPLER_CLAMPED,
	TEXTURE_SAMPLER_LINEAR,
	TEXTURE_SAMPLER_LINEAR_CLAMPED,
	TEXTURE_SAMPLER_LINEAR_CLAMPED_VERTICAL,
	TEXTURE_SAMPLER_SHADOW_LINEAR_CLAMPED,

	TEXTURE_SAMPLER_COUNT
};

struct Texture
{
	SDL_GPUTexture* handle;
	TextureInfo info;
};


Texture* LoadTexture(const char* path, SDL_GPUCommandBuffer* cmdBuffer);
Texture* LoadTextureFromData(const uint8_t* data, uint32_t size, const TextureInfo* info, SDL_GPUCommandBuffer* cmdBuffer);

void DestroyTexture(Texture* texture);
