#include "IndexBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


IndexBuffer* CreateIndexBuffer(int numIndices, SDL_GPUIndexElementSize elementSize, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &bufferInfo);

	SDL_GPUTransferBufferCreateInfo transferInfo = {};
	transferInfo.size = size;
	transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

	SDL_assert(numIndices * GetIndexFormatSize(elementSize) == size);

	void* vertexData = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
	SDL_memcpy(vertexData, data, size);
	SDL_UnmapGPUTransferBuffer(device, transferBuffer);

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	SDL_GPUTransferBufferLocation location = {};
	location.transfer_buffer = transferBuffer;
	location.offset = 0;

	SDL_GPUBufferRegion region = {};
	region.buffer = buffer;
	region.size = size;
	region.offset = 0;

	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
	SDL_EndGPUCopyPass(copyPass);

	SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

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

uint32_t GetIndexFormatSize(SDL_GPUIndexElementSize elementSize)
{
	return elementSize == SDL_GPU_INDEXELEMENTSIZE_16BIT ? 2 : 4;
}
