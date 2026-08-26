#include "StorageBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


TransferBuffer* CreateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage)
{
	SDL_GPUTransferBufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	SDL_GPUTransferBuffer* buffer = SDL_CreateGPUTransferBuffer(device, &bufferInfo);

	TransferBuffer* transferBuffer = PoolAlloc(&graphics->transferBuffers);
	transferBuffer->buffer = buffer;

	return transferBuffer;
}

void DestroyTransferBuffer(TransferBuffer* transferBuffer)
{
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer->buffer);
	PoolRelease(&graphics->transferBuffers, transferBuffer);
}

void* MapTransferBuffer(TransferBuffer* transferBuffer, bool cycle)
{
	return SDL_MapGPUTransferBuffer(device, transferBuffer->buffer, cycle);
}

void UnmapTransferBuffer(TransferBuffer* transferBuffer)
{
	SDL_UnmapGPUTransferBuffer(device, transferBuffer->buffer);
}
