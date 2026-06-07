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

struct GUIRenderer
{
	int maxSprites;

	SDL_GPUBuffer* spriteDataBuffer;
	SDL_GPUTransferBuffer* transferBuffer;

#define MAX_GUIRenderer_TEXTURES 16
	Texture* textures[MAX_GUIRenderer_TEXTURES];
	int numTextures;

	int numSprites;
	SpriteData* spriteDataPtr = nullptr;

	Shader* spriteShader;
	GraphicsPipeline* spritePipeline;

	SDL_GPUSampler* sampler;
	SDL_GPUTexture* emptyTexture;

	mat4 pv;
};


void InitGUIRenderer(GUIRenderer* renderer, int maxSprites, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyGUIRenderer(GUIRenderer* renderer);

void BeginGUIRenderer(GUIRenderer* renderer, mat4 pv);
void DrawSprite(GUIRenderer* renderer, const SpriteDrawData* drawData);
void EndGUIRenderer(GUIRenderer* renderer, SDL_GPUCommandBuffer* cmdBuffer);
