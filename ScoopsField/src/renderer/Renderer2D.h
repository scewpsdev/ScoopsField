#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "graphics/RenderTarget.h"
#include "graphics/GraphicsPipeline.h"

#include "ScreenQuad.h"

#include <SDL3/SDL.h>


struct Renderer2DLayerInfo
{
	int width, height;
	int maxSprites;
	SDL_GPUTextureFormat textureFormat;
};

struct SpriteDrawData
{
	vec3 position;
	float rotation;
	vec2 size;
	vec4 rect;
	vec4 color;
	Texture* texture;
};

struct SpriteData
{
	vec3 position;
	float rotation;
	vec2 size;
	int textureID;
	float padding;
	vec4 rect;
	vec4 color;
};

struct VertexUniforms
{
	mat4 projectionView;
};

struct Renderer2DLayer
{
	Renderer2DLayerInfo info;
	RenderTarget* renderTarget;

	SDL_GPUBuffer* spriteDataBuffer;
	SDL_GPUTransferBuffer* transferBuffer;

#define MAX_RENDERER2D_LAYER_TEXTURES 16
	Texture* textures[MAX_RENDERER2D_LAYER_TEXTURES];
	int numTextures;

	VertexUniforms vertexUniforms;

	int numSprites;
	SpriteData* spriteDataPtr = nullptr;
};

struct Renderer2D
{
	Shader* spriteShader;
	GraphicsPipeline* spritePipeline;

#define MAX_RENDERER2D_LAYERS 8
	Renderer2DLayer layers[MAX_RENDERER2D_LAYERS];
	int numLayers;

	ScreenQuad screenQuad;
	Shader* quadShader;
	GraphicsPipeline* quadPipeline;

	SDL_GPUSampler* sampler;
	SDL_GPUTexture* emptyTexture;
};


void InitRenderer2D(Renderer2D* renderer, int numLayers, Renderer2DLayerInfo* layerInfo, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyRenderer2D(Renderer2D* renderer);

void ResizeRenderer2D(Renderer2D* renderer, int width, int height);

void SetRenderer2DCamera(Renderer2D* renderer, int layerID, mat4 projectionView);

void BeginRenderer2D(Renderer2D* renderer);
void DrawSprite(Renderer2D* renderer, int layerID, const SpriteDrawData* drawData);
void EndRenderer2D(Renderer2D* renderer, SDL_GPUCommandBuffer* cmdBuffer);
