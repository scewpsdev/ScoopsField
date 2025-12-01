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

	return transferBuffer;
}

void DestroyTransferBuffer(TransferBuffer* transferBuffer)
{
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer->buffer);
}
