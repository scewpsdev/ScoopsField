#pragma once

#include <SDL3/SDL.h>


struct TransferBuffer
{
	SDL_GPUTransferBuffer* buffer;
	bool cycle;
	void* mapped;
};


TransferBuffer* CreateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage, bool cycle);
void DestroyTransferBuffer(TransferBuffer* transferBuffer);

void* MapTransferBuffer(TransferBuffer* transferBuffer);
void UnmapTransferBuffer(TransferBuffer* transferBuffer);
