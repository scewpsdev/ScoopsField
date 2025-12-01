#pragma once

#include <SDL3/SDL.h>


struct StorageBuffer
{
	SDL_GPUBuffer* buffer;
};


StorageBuffer* CreateStorageBuffer(const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyStorageBuffer(StorageBuffer* storageBuffer);

void UpdateStorageBuffer(StorageBuffer* storageBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUTransferBuffer* transferBuffer, bool cycleTransferBuffer, SDL_GPUCommandBuffer* cmdBuffer);
