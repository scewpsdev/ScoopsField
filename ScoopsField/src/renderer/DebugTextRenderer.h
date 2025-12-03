#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "graphics/RenderTarget.h"

#include <SDL3/SDL.h>


struct GlyphData
{
	vec2 position;
	vec2 padding;
	vec4 rect;
	vec4 color;
	vec4 bgcolor;
};

struct DebugTextRenderer
{
	int maxGlyphs;

	Shader* shader;
	Texture* texture;

	SDL_GPUGraphicsPipeline* pipeline;
	SDL_GPUSampler* sampler;

	SDL_GPUBuffer* glyphDataBuffer;
	SDL_GPUTransferBuffer* transferBuffer;

	int numGlyphs;
	GlyphData* glyphDataPtr;
};


void InitDebugTextRenderer(DebugTextRenderer* renderer, int maxGlyphs, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyDebugTextRenderer(DebugTextRenderer* renderer);

void DebugTextRendererBegin(DebugTextRenderer* renderer);
void DebugTextRendererSubmit(DebugTextRenderer* renderer, int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor);
void DebugTextRendererEnd(DebugTextRenderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer);
