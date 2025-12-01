#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "graphics/RenderTarget.h"

#include "ScreenQuad.h"

#include <SDL3/SDL.h>


struct Renderer2DLayerInfo
{
	int width, height;
	int maxSprites;
};

struct SpriteDrawData
{
	vec3 position;
	float rotation;
	vec2 size;
	vec4 rect;
	vec4 color;
};

struct SpriteData
{
	vec3 position;
	float rotation;
	vec2 size;
	vec2 padding;
	vec4 rect;
	vec4 color;
};

struct VertexUniforms
{
	Matrix projectionView;
};

struct Renderer2DLayer
{
	Renderer2DLayerInfo info;
	RenderTarget* renderTarget;

	SDL_GPUBuffer* spriteDataBuffer;
	SDL_GPUTransferBuffer* transferBuffer;

	VertexUniforms vertexUniforms;

	int numSprites;
	SpriteData* spriteDataPtr = nullptr;
};

struct Renderer2D
{
	Shader* triangleShader;
	Texture* texture;

	SDL_GPUGraphicsPipeline* pipeline;
	SDL_GPUSampler* sampler;

	int numLayers;
	Renderer2DLayer* layers;

	ScreenQuad screenQuad;
	Shader* quadShader;
	SDL_GPUGraphicsPipeline* quadPipeline;
};


void InitRenderer2D(Renderer2D* renderer, int numLayers, Renderer2DLayerInfo* layerInfo, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyRenderer2D(Renderer2D* renderer);

void Renderer2DSetProjectionView(Renderer2D* renderer, int layerID, Matrix projectionView);

void Renderer2DBegin(Renderer2D* renderer);
void Renderer2DDrawSprite(Renderer2D* renderer, int layerID, SpriteDrawData drawData);
void Renderer2DEnd(Renderer2D* renderer, SDL_GPUCommandBuffer* cmdBuffer);
