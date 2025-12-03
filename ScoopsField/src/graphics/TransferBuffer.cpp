#include "StorageBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


TransferBuffer* CreateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage, bool cycle)
{
	SDL_GPUTransferBufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	SDL_GPUTransferBuffer* buffer = SDL_CreateGPUTransferBuffer(device, &bufferInfo);

	SDL_assert(graphics->numTransferBuffers < MAX_TRANSFER_BUFFERS);

	TransferBuffer* transferBuffer = &graphics->transferBuffers[graphics->numTransferBuffers++];
	transferBuffer->buffer = buffer;
	transferBuffer->cycle = cycle;
	transferBuffer->mapped = nullptr;

	return transferBuffer;
}

void DestroyTransferBuffer(TransferBuffer* transferBuffer)
{
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer->buffer);
}

void* MapTransferBuffer(TransferBuffer* transferBuffer)
{
	return transferBuffer->mapped = SDL_MapGPUTransferBuffer(device, transferBuffer->buffer, transferBuffer->cycle);
}

void UnmapTransferBuffer(TransferBuffer* transferBuffer)
{
	SDL_UnmapGPUTransferBuffer(device, transferBuffer->buffer);
	transferBuffer->mapped = nullptr;
}
