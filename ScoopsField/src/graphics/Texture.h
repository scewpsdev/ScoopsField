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

struct Texture
{
	SDL_GPUTexture* handle;
	TextureInfo info;
};


Texture* LoadTexture(const char* path, SDL_GPUCommandBuffer* cmdBuffer);

void DestroyTexture(Texture* texture);
