#include "VertexBuffer.h"

#include "Application.h"

#include <SDL3/SDL.h>


static const uint32_t vertexElementFormatSizes[SDL_GPU_VERTEXELEMENTFORMAT_HALF4 + 1] = {
	/* SDL_GPU_VERTEXELEMENTFORMAT_INVALID            */ 0,
	/* SDL_GPU_VERTEXELEMENTFORMAT_INT                */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_INT2               */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_INT3               */ 12,
	/* SDL_GPU_VERTEXELEMENTFORMAT_INT4               */ 16,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UINT               */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UINT2              */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UINT3              */ 12,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UINT4              */ 16,
	/* SDL_GPU_VERTEXELEMENTFORMAT_FLOAT              */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2             */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3             */ 12,
	/* SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4             */ 16,
	/* SDL_GPU_VERTEXELEMENTFORMAT_BYTE2              */ 2,
	/* SDL_GPU_VERTEXELEMENTFORMAT_BYTE4              */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2             */ 2,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4             */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM         */ 2,
	/* SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM         */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM        */ 2,
	/* SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM        */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_SHORT2             */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_SHORT4             */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_USHORT2            */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_USHORT4            */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM        */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM        */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM       */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM       */ 8,
	/* SDL_GPU_VERTEXELEMENTFORMAT_HALF2              */ 4,
	/* SDL_GPU_VERTEXELEMENTFORMAT_HALF4              */ 8
};


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


VertexBuffer* CreateVertexBuffer(int numVertices, const VertexBufferLayout* layout, SDL_GPUBufferUsageFlags usageFlags, const uint8_t* data, uint32_t size, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.size = numVertices * GetVertexPitch(layout);
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX | usageFlags;
	SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &bufferInfo);

	if (data)
	{
		SDL_GPUTransferBufferCreateInfo transferInfo = {};
		transferInfo.size = size;
		transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

		SDL_assert(size <= bufferInfo.size);

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
	}

	SDL_assert(graphics->numVertexBuffers < MAX_VERTEX_BUFFERS);

	VertexBuffer* vertexBuffer = &graphics->vertexBuffers[graphics->numVertexBuffers++];
	vertexBuffer->layout = *layout;
	vertexBuffer->numVertices = numVertices;
	vertexBuffer->buffer = buffer;

	return vertexBuffer;
}

void DestroyVertexBuffer(VertexBuffer* vertexBuffer)
{
	SDL_ReleaseGPUBuffer(device, vertexBuffer->buffer);
}

void UpdateVertexBuffer(VertexBuffer* vertexBuffer, uint32_t offset, uint8_t* data, uint32_t size, SDL_GPUTransferBuffer* transferBuffer, bool cycleTransferBuffer, SDL_GPUCommandBuffer* cmdBuffer)
{
	void* mappedBuffer = SDL_MapGPUTransferBuffer(device, transferBuffer, cycleTransferBuffer);
	SDL_memcpy(mappedBuffer, data, size);
	SDL_UnmapGPUTransferBuffer(device, transferBuffer);

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	SDL_GPUTransferBufferLocation location = {};
	location.transfer_buffer = transferBuffer;
	location.offset = 0;

	SDL_GPUBufferRegion region = {};
	region.buffer = vertexBuffer->buffer;
	region.size = size;
	region.offset = offset;

	SDL_assert(offset + size <= vertexBuffer->numVertices * GetVertexPitch(&vertexBuffer->layout));

	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
	SDL_EndGPUCopyPass(copyPass);
}

uint32_t GetVertexPitch(const VertexBufferLayout* layout)
{
	uint32_t pitch = 0;
	for (int i = 0; i < layout->numAttributes; i++)
		pitch += GetVertexFormatSize(layout->attributes[i].format);
	return pitch;
}

uint32_t GetVertexFormatSize(SDL_GPUVertexElementFormat format)
{
	return vertexElementFormatSizes[format];
}
