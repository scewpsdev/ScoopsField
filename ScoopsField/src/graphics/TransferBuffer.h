#pragma once

#include <SDL3/SDL.h>


struct TransferBuffer
{
	SDL_GPUTransferBuffer* buffer;
};


TransferBuffer* CreateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage);
void DestroyTransferBuffer(TransferBuffer* transferBuffer);

void* MapTransferBuffer(TransferBuffer* transferBuffer, bool cycle);
void UnmapTransferBuffer(TransferBuffer* transferBuffer);
