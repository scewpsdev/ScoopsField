#include "ScreenQuad.h"

#include "math/Vector.h"


extern SDL_Window* window;
extern SDL_GPUDevice* device;


static const vec2 vertices[] = {
	vec2(-3, -1),
	vec2(1, -1),
	vec2(1, 3)
};


void InitScreenQuad(ScreenQuad* quad, SDL_GPUCommandBuffer* cmdBuffer)
{
	VertexBufferLayout bufferLayout = {};
	bufferLayout.numAttributes = 1;
	bufferLayout.attributes[0].location = 0;
	bufferLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	quad->vertexBuffer = CreateVertexBuffer(3, &bufferLayout, 0, (uint8_t*)vertices, sizeof(vertices), cmdBuffer);
}

void DestroyScreenQuad(ScreenQuad* quad)
{
	DestroyVertexBuffer(quad->vertexBuffer);
}

void RenderScreenQuad(ScreenQuad* quad, SDL_GPURenderPass* renderPass, int numTextures, SDL_GPUTexture** textures, SDL_GPUSampler* sampler, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUBufferBinding bufferBindings[1];
	bufferBindings[0].buffer = quad->vertexBuffer->buffer;
	bufferBindings[0].offset = 0;

	SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);

	SDL_GPUTextureSamplerBinding textureBindings[8];
	for (int i = 0; i < numTextures; i++)
	{
		textureBindings[i].texture = textures[i];
		textureBindings[i].sampler = sampler;
	}

	SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, numTextures);

	SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
}
