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


IndexBuffer* CreateIndexBuffer(int numIndices, SDL_GPUIndexElementSize elementSize);
void DestroyIndexBuffer(IndexBuffer* indexBuffer);

void UpdateIndexBuffer(IndexBuffer* indexBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUTransferBuffer* transferBuffer, SDL_GPUCopyPass* copyPass);
void UpdateIndexBuffer(IndexBuffer* indexBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUCopyPass* copyPass);
void UpdateIndexBuffer(IndexBuffer* indexBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer);

inline uint32_t GetIndexFormatSize(SDL_GPUIndexElementSize elementSize) { return elementSize == SDL_GPU_INDEXELEMENTSIZE_16BIT ? sizeof(int16_t) : sizeof(int32_t); }
