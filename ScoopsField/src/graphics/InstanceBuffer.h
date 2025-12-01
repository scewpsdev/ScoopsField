#pragma once

#include <SDL3/SDL.h>


struct InstanceBuffer
{
	int numInstances;
	uint32_t instanceSize;
	SDL_GPUBuffer* buffer;
};


InstanceBuffer* CreateInstanceBuffer(int numInstances, uint32_t instanceSize, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyInstanceBuffer(InstanceBuffer* vertexBuffer);
