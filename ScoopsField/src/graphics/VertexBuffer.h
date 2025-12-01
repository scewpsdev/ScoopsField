#pragma once

#include <SDL3/SDL.h>


struct VertexAttribute
{
	uint32_t location;
	SDL_GPUVertexElementFormat format;
};

struct VertexBufferLayout
{
#define MAX_VERTEX_ATTRIBUTES 8
	int numAttributes;
	VertexAttribute attributes[MAX_VERTEX_ATTRIBUTES];
	bool perInstance;
};

struct VertexBuffer
{
	int numVertices;
	VertexBufferLayout layout;
	SDL_GPUBuffer* buffer;
};


VertexBuffer* CreateVertexBuffer(int numVertices, const VertexBufferLayout* layout, SDL_GPUBufferUsageFlags usageFlags, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyVertexBuffer(VertexBuffer* vertexBuffer);

void UpdateVertexBuffer(VertexBuffer* vertexBuffer, uint32_t offset, uint8_t* data, uint32_t size, SDL_GPUTransferBuffer* transferBuffer, bool cycleTransferBuffer, SDL_GPUCommandBuffer* cmdBuffer);

uint32_t GetVertexPitch(const VertexBufferLayout* layout);
uint32_t GetVertexFormatSize(SDL_GPUVertexElementFormat format);
