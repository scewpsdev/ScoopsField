#pragma once

#include "ScreenQuad.h"

#include "graphics/RenderTarget.h"
#include "graphics/Shader.h"
#include "graphics/GraphicsPipeline.h"

#include <SDL3/SDL.h>


struct Renderer
{
	int width, height;

	SDL_GPUTexture* depthTexture;
	RenderTarget* gbuffer;
	Shader* lightingShader;
	GraphicsPipeline* lightingPipeline;

	ScreenQuad screenQuad;
	SDL_GPUSampler* defaultSampler;
};


void InitRenderer(Renderer* renderer, int width, int height);
void DestroyRenderer(Renderer* renderer);
void ResizeRenderer(Renderer* renderer, int width, int height);

void RenderScene(Renderer* renderer, struct GameState* game, SDL_GPUCommandBuffer* cmdBuffer);
