#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"

#include "ScreenQuad.h"

#include <SDL3/SDL.h>


struct Shader;
struct GraphicsPipeline;
struct Texture;
struct Font;

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

struct SpriteRenderer
{
	int maxSprites;

	SDL_GPUBuffer* spriteDataBuffer;
	SDL_GPUTransferBuffer* transferBuffer;

#define MAX_GUI_RENDERER_TEXTURES 16
	SDL_GPUTexture* textures[MAX_GUI_RENDERER_TEXTURES];
	int numTextures;

	int numSprites;
	SpriteData* spriteDataPtr = nullptr;

	GraphicsPipeline* shader;

	SDL_GPUSampler* sampler;
	SDL_GPUTexture* emptyTexture;

	mat4 pv;
};


void InitSpriteRenderer(SpriteRenderer* renderer, int maxSprites, GraphicsPipeline* shader, SDL_GPUCommandBuffer* cmdBuffer);
void DestroySpriteRenderer(SpriteRenderer* renderer);

void BeginSpriteRenderer(SpriteRenderer* renderer, mat4 pv);
void DrawSprite(SpriteRenderer* renderer, const SpriteDrawData* drawData);
void DrawText(SpriteRenderer* renderer, float x, float y, const char* text, int length, Font* font, uint32_t color);
void EndSpriteRenderer(SpriteRenderer* renderer, SDL_GPUCommandBuffer* cmdBuffer);
