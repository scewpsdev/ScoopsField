#include "IndexBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


IndexBuffer* CreateIndexBuffer(int numIndices, SDL_GPUIndexElementSize elementSize)
{
	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.size = numIndices * GetIndexFormatSize(elementSize);
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &bufferInfo);

	SDL_assert(graphics->numIndexBuffers < MAX_INDEX_BUFFERS);

	IndexBuffer* indexBuffer = &graphics->indexBuffers[graphics->numIndexBuffers++];
	indexBuffer->buffer = buffer;
	indexBuffer->numIndices = numIndices;
	indexBuffer->elementSize = elementSize;

	return indexBuffer;
}

void DestroyIndexBuffer(IndexBuffer* indexBuffer)
{
	SDL_ReleaseGPUBuffer(device, indexBuffer->buffer);
}

void UpdateIndexBuffer(IndexBuffer* indexBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUTransferBuffer* transferBuffer, SDL_GPUCopyPass* copyPass)
{
	SDL_assert(size <= indexBuffer->numIndices * GetIndexFormatSize(indexBuffer->elementSize));

	SDL_GPUTransferBufferLocation location = {};
	location.transfer_buffer = transferBuffer;
	location.offset = 0;

	SDL_GPUBufferRegion region = {};
	region.buffer = indexBuffer->buffer;
	region.size = size;
	region.offset = offset;

	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
}

void UpdateIndexBuffer(IndexBuffer* indexBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUCopyPass* copyPass)
{
	SDL_GPUTransferBufferCreateInfo transferInfo = {};
	transferInfo.size = size;
	transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
	void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
	SDL_memcpy(mapped, data, size);
	SDL_UnmapGPUTransferBuffer(device, transferBuffer);
	UpdateIndexBuffer(indexBuffer, offset, data, size, transferBuffer, copyPass);
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
}

void UpdateIndexBuffer(IndexBuffer* indexBuffer, uint32_t offset, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	UpdateIndexBuffer(indexBuffer, offset, data, size, copyPass);

	SDL_EndGPUCopyPass(copyPass);
}
