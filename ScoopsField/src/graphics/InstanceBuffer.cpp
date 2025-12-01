#include "InstanceBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


InstanceBuffer* CreateInstanceBuffer(int numInstances, uint32_t instanceSize, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &bufferInfo);

	SDL_GPUTransferBufferCreateInfo transferInfo = {};
	transferInfo.size = size;
	transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

	SDL_assert(numInstances * instanceSize == size);

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

	SDL_assert(graphics->numInstanceBuffers < MAX_INSTANCE_BUFFERS);

	InstanceBuffer* instanceBuffer = &graphics->instanceBuffers[graphics->numInstanceBuffers++];
	instanceBuffer->numInstances = numInstances;
	instanceBuffer->instanceSize = instanceSize;
	instanceBuffer->buffer = buffer;

	return instanceBuffer;
}

void DestroyInstanceBuffer(InstanceBuffer* instanceBuffer)
{
	SDL_ReleaseGPUBuffer(device, instanceBuffer->buffer);
}
