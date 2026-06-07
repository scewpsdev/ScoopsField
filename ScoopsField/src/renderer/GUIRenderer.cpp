#include "GUIRenderer.h"

#include "math/Vector.h"
#include "math/Matrix.h"

#include <SDL3/SDL.h>



extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern SDL_GPUTexture* swapchain;


struct VertexUniforms
{
	mat4 projectionView;
};


void InitGUIRenderer(GUIRenderer* renderer, int maxSprites, SDL_GPUCommandBuffer* cmdBuffer)
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

	renderer->spriteShader = LoadGraphicsShader("res/shaders/sprite.vert.bin", "res/shaders/sprite.frag.bin");

	{
		VertexBufferLayout spriteVertexLayout = {};
		spriteVertexLayout.numAttributes = 1;
		spriteVertexLayout.attributes[0].location = 0;
		spriteVertexLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

		GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(
			SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			SDL_GPU_CULLMODE_BACK,
			renderer->spriteShader,
			nullptr,
			1, &spriteVertexLayout
		);

		CreateBlendStateAlphaPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

		renderer->spritePipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

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

void DestroyGUIRenderer(GUIRenderer* renderer)
{
	SDL_ReleaseGPUSampler(device, renderer->sampler);
	SDL_ReleaseGPUTexture(device, renderer->emptyTexture);

	DestroyGraphicsPipeline(renderer->spritePipeline);
	DestroyShader(renderer->spriteShader);

	SDL_ReleaseGPUBuffer(device, renderer->spriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, renderer->transferBuffer);
}

void BeginGUIRenderer(GUIRenderer* renderer, mat4 pv)
{
	renderer->pv = pv;

	renderer->spriteDataPtr = (SpriteData*)SDL_MapGPUTransferBuffer(device, renderer->transferBuffer, true);
	renderer->numSprites = 0;
	renderer->numTextures = 0;
}

static int GetTextureID(GUIRenderer* renderer, Texture* texture)
{
	if (!texture)
		return -1;

	SDL_assert(renderer->numTextures < MAX_GUIRenderer_TEXTURES);

	int id = renderer->numTextures++;
	renderer->textures[id] = texture;
	return id;
}

void DrawSprite(GUIRenderer* renderer, const SpriteDrawData* drawData)
{
	SDL_assert(renderer->numSprites < renderer->maxSprites);

	SpriteData* spriteData = &renderer->spriteDataPtr[renderer->numSprites++];

	spriteData->position = drawData->position;
	spriteData->rotation = drawData->rotation;
	spriteData->size = drawData->size;
	spriteData->rect = drawData->rect;
	spriteData->color = drawData->color;
	spriteData->textureID = GetTextureID(renderer, drawData->texture);
}

void EndGUIRenderer(GUIRenderer* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (renderer->numSprites == 0)
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

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->spritePipeline->pipeline);

		SDL_BindGPUVertexStorageBuffers(renderPass, 0, &renderer->spriteDataBuffer, 1);

		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &renderer->pv, sizeof(renderer->pv));

		SDL_GPUTextureSamplerBinding textureBindings[MAX_GUIRenderer_TEXTURES] = {};
		for (int i = 0; i < MAX_GUIRenderer_TEXTURES; i++)
		{
			textureBindings[i].texture = i < renderer->numTextures ? renderer->textures[i]->handle : renderer->emptyTexture;
			textureBindings[i].sampler = renderer->sampler;
		}
		SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, MAX_GUIRenderer_TEXTURES);

		SDL_DrawGPUPrimitives(renderPass, renderer->numSprites * 6, 1, 0, 0);

		SDL_EndGPURenderPass(renderPass);
	}
}
