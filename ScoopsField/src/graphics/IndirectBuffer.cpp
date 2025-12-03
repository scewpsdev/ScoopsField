#include "IndirectBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


IndirectBuffer* CreateIndirectBuffer(int maxDrawCommands, bool indexed, SDL_GPUBufferUsageFlags usage)
{
	uint32_t drawCommandSize = indexed ? sizeof(SDL_GPUIndexedIndirectDrawCommand) : sizeof(SDL_GPUIndirectDrawCommand);

	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.size = maxDrawCommands * drawCommandSize;
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDIRECT | usage;
	SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &bufferInfo);

	SDL_assert(graphics->numIndirectBuffers < MAX_INDIRECT_BUFFERS);

	IndirectBuffer* indirectBuffer = &graphics->indirectBuffers[graphics->numIndirectBuffers++];
	indirectBuffer->maxDrawCommands = maxDrawCommands;
	indirectBuffer->indexed = indexed;
	indirectBuffer->buffer = buffer;

	return indirectBuffer;
}

void UpdateIndirectBuffer(IndirectBuffer* buffer, uint32_t firstDrawCommand, const SDL_GPUIndirectDrawCommand* drawCommands, int numDrawCommands, SDL_GPUTransferBuffer* transferBuffer, bool cycleTransferBuffer, SDL_GPUCommandBuffer* cmdBuffer)
{
	// TODO pack chunk vertex buffer tightly, for later chunk updates create a custom allocator that finds space in the gaps.

	uint32_t drawCommandSize = buffer->indexed ? sizeof(SDL_GPUIndexedIndirectDrawCommand) : sizeof(SDL_GPUIndirectDrawCommand);

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	void* mappedBuffer = SDL_MapGPUTransferBuffer(device, transferBuffer, cycleTransferBuffer);
	SDL_memcpy(mappedBuffer, drawCommands, numDrawCommands * drawCommandSize);
	SDL_UnmapGPUTransferBuffer(device, transferBuffer);

	SDL_GPUTransferBufferLocation location = {};
	location.transfer_buffer = transferBuffer;
	location.offset = 0;

	SDL_GPUBufferRegion region = {};
	region.buffer = buffer->buffer;
	region.size = numDrawCommands * drawCommandSize;
	region.offset = firstDrawCommand * drawCommandSize;

	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
	SDL_EndGPUCopyPass(copyPass);
}

void DestroyIndirectBuffer(IndirectBuffer* indirectBuffer)
{
	SDL_ReleaseGPUBuffer(device, indirectBuffer->buffer);
}
