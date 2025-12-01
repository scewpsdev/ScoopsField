#pragma once

#include <SDL3/SDL.h>


struct IndirectBuffer
{
	int maxDrawCommands;
	bool indexed;
	SDL_GPUBuffer* buffer;
};


IndirectBuffer* CreateIndirectBuffer(int maxDrawCommands, bool indexed, SDL_GPUBufferUsageFlags usage);
void UpdateIndirectBuffer(IndirectBuffer* buffer, uint32_t firstDrawCommand, const SDL_GPUIndirectDrawCommand* drawCommands, int numDrawCommands, SDL_GPUTransferBuffer* transferBuffer, bool cycleTransferBuffer, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyIndirectBuffer(IndirectBuffer* indirectBuffer);
