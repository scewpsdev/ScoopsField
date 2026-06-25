#include "SpriteRenderer.h"

#include "Application.h"

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "graphics/RenderTarget.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/Font.h"

#include "math/Math.h"
#include "math/Vector.h"
#include "math/Matrix.h"

#include <SDL3/SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>



extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern SDL_GPUTexture* swapchain;


struct VertexUniforms
{
	mat4 projectionView;
};


void InitSpriteRenderer(SpriteRenderer* renderer, int maxSprites, GraphicsPipeline* shader, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->maxSprites = maxSprites;

	SDL_GPUBufferCreateInfo spriteDataBufferInfo = {};
	spriteDataBufferInfo.size = maxSprites * sizeof(SpriteData);
	spriteDataBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;

	renderer->spriteDataBuffer = SDL_CreateGPUBuffer(device, &spriteDataBufferInfo);

	SDL_GPUTransferBufferCreateInfo transferBufferInfo = {};
	transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBufferInfo.size = maxSprites * sizeof(SpriteData);

	renderer->transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

	renderer->shader = shader;

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->sampler = SDL_CreateGPUSampler(device, &samplerInfo);

	SDL_GPUTextureCreateInfo emptyTextureInfo = {};
	emptyTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	emptyTextureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	emptyTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	emptyTextureInfo.width = 1;
	emptyTextureInfo.height = 1;
	emptyTextureInfo.layer_count_or_depth = 1;
	emptyTextureInfo.num_levels = 1;
	emptyTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	renderer->emptyTexture = SDL_CreateGPUTexture(device, &emptyTextureInfo);
}

void DestroySpriteRenderer(SpriteRenderer* renderer)
{
	SDL_ReleaseGPUSampler(device, renderer->sampler);
	SDL_ReleaseGPUTexture(device, renderer->emptyTexture);

	SDL_ReleaseGPUBuffer(device, renderer->spriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, renderer->transferBuffer);
}

void BeginSpriteRenderer(SpriteRenderer* renderer, mat4 pv)
{
	renderer->pv = pv;

	renderer->spriteDataPtr = (SpriteData*)SDL_MapGPUTransferBuffer(device, renderer->transferBuffer, true);
	renderer->numSprites = 0;
	renderer->numTextures = 0;
}

static int GetTextureID(SpriteRenderer* renderer, SDL_GPUTexture* texture)
{
	if (!texture)
		return -1;

	for (int i = 0; i < renderer->numTextures; i++)
	{
		if (renderer->textures[i] == texture)
			return i;
	}

	SDL_assert(renderer->numTextures < MAX_GUI_RENDERER_TEXTURES);

	int id = renderer->numTextures++;
	renderer->textures[id] = texture;
	return id;
}

void DrawSprite(SpriteRenderer* renderer, const SpriteDrawData* drawData)
{
	SDL_assert(renderer->numSprites < renderer->maxSprites);

	SpriteData* spriteData = &renderer->spriteDataPtr[renderer->numSprites++];

	spriteData->position = drawData->position;
	spriteData->rotation = drawData->rotation;
	spriteData->size = drawData->size;
	spriteData->rect = drawData->rect;
	spriteData->color = drawData->color;
	spriteData->textureID = GetTextureID(renderer, drawData->texture ? drawData->texture->handle : nullptr);
}

void DrawText(SpriteRenderer* renderer, float x, float y, const char* text, int length, Font* font, uint32_t color)
{
	for (int i = 0; i < length; i++)
	{
		int charIndex = text[i] - FONT_CHAR_OFFSET;
		stbtt_aligned_quad quad;
		stbtt_GetBakedQuad((const stbtt_bakedchar*)font->characters, font->width, font->height, charIndex, &x, &y, &quad, 1);

		uint8_t r = (color & 0xFF0000) >> 16;
		uint8_t g = (color & 0xFF00) >> 8;
		uint8_t b = (color & 0xFF);
		uint8_t a = (color & 0xFF000000) >> 24;

		SDL_assert(renderer->numSprites < renderer->maxSprites);

		SpriteData* spriteData = &renderer->spriteDataPtr[renderer->numSprites++];

		spriteData->position = vec3(mix(quad.x0, quad.x1, 0.5f), app->height - mix(quad.y0, quad.y1, 0.5f), 0);
		spriteData->rotation = 0;
		spriteData->size = vec2(quad.x1 - quad.x0, quad.y1 - quad.y0);
		spriteData->textureID = GetTextureID(renderer, font->texture);
		spriteData->rect = vec4(quad.s0, quad.t0, quad.s1 - quad.s0, quad.t1 - quad.t0);
		spriteData->color = ARGBToVector(color);
	}
}

void EndSpriteRenderer(SpriteRenderer* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (renderer->numSprites == 0)
		return;

	if (!swapchain)
		return;

	{
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		SDL_UnmapGPUTransferBuffer(device, renderer->transferBuffer);
		renderer->spriteDataPtr = nullptr;

		SDL_GPUTransferBufferLocation location = {};
		location.transfer_buffer = renderer->transferBuffer;
		location.offset = 0;

		SDL_GPUBufferRegion region = {};
		region.buffer = renderer->spriteDataBuffer;
		region.size = renderer->numSprites * sizeof(SpriteData);
		region.offset = 0;

		SDL_UploadToGPUBuffer(copyPass, &location, &region, true);

		SDL_EndGPUCopyPass(copyPass);
	}

	{
		SDL_GPUColorTargetInfo colorTargetInfo = {};
		colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		colorTargetInfo.texture = swapchain;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTargetInfo, 1, nullptr);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->shader->pipeline);

		SDL_BindGPUVertexStorageBuffers(renderPass, 0, &renderer->spriteDataBuffer, 1);

		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &renderer->pv, sizeof(renderer->pv));

		SDL_GPUTextureSamplerBinding textureBindings[MAX_GUI_RENDERER_TEXTURES] = {};
		for (int i = 0; i < MAX_GUI_RENDERER_TEXTURES; i++)
		{
			textureBindings[i].texture = i < renderer->numTextures ? renderer->textures[i] : renderer->emptyTexture;
			textureBindings[i].sampler = renderer->sampler;
		}
		SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, MAX_GUI_RENDERER_TEXTURES);

		SDL_DrawGPUPrimitives(renderPass, renderer->numSprites * 6, 1, 0, 0);

		SDL_EndGPURenderPass(renderPass);
	}
}
