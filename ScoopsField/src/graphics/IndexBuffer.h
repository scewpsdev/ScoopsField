#pragma once

#include <SDL3/SDL.h>


struct IndexBufferData
{
	const void* data;
	uint32_t size;
};

struct IndexBuffer
{
	int numIndices;
	SDL_GPUIndexElementSize elementSize;
	SDL_GPUBuffer* buffer;
};


IndexBuffer* CreateIndexBuffer(int numIndices, SDL_GPUIndexElementSize elementSize, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyIndexBuffer(IndexBuffer* indexBuffer);

uint32_t GetIndexFormatSize(SDL_GPUIndexElementSize elementSize);
