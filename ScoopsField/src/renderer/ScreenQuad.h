#pragma once

#include "graphics/VertexBuffer.h"
#include "graphics/Shader.h"

#include <SDL3/SDL.h>


struct ScreenQuad
{
	VertexBuffer* vertexBuffer;
};


void InitScreenQuad(ScreenQuad* quad, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyScreenQuad(ScreenQuad* quad);
void RenderScreenQuad(ScreenQuad* quad, SDL_GPURenderPass* renderPass, int numTextures, SDL_GPUTexture** textures, SDL_GPUSampler* sampler, SDL_GPUCommandBuffer* cmdBuffer);
