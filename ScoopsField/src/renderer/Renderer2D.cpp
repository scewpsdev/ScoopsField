#include "Renderer2D.h"

#include "math/Vector.h"
#include "math/Matrix.h"

#include <SDL3/SDL.h>



extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern SDL_GPUTexture* swapchain;


static void InitLayer(Renderer2DLayer* layer, Renderer2DLayerInfo* layerInfo)
{
	layer->info = *layerInfo;

	ColorAttachmentInfo colorAttachment = {};
	colorAttachment.format = layerInfo->textureFormat;
	colorAttachment.loadOp = SDL_GPU_LOADOP_CLEAR;
	colorAttachment.storeOp = SDL_GPU_STOREOP_STORE;
	colorAttachment.clearColor = vec4(0.0f);
	layer->renderTarget = CreateRenderTarget(layerInfo->width, layerInfo->height, SDL_GPU_TEXTURETYPE_2D, 1, &colorAttachment, nullptr);

	SDL_GPUBufferCreateInfo spriteDataBufferInfo = {};
	spriteDataBufferInfo.size = layerInfo->maxSprites * sizeof(SpriteData);
	spriteDataBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;

	layer->spriteDataBuffer = SDL_CreateGPUBuffer(device, &spriteDataBufferInfo);

	SDL_GPUTransferBufferCreateInfo transferBufferInfo = {};
	transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBufferInfo.size = layerInfo->maxSprites * sizeof(SpriteData);

	layer->transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

	layer->vertexUniforms.projectionView = mat4::Identity;
}

void InitRenderer2D(Renderer2D* renderer, int numLayers, Renderer2DLayerInfo* layerInfo, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->numLayers = numLayers;
	for (int i = 0; i < numLayers; i++)
	{
		InitLayer(&renderer->layers[i], &layerInfo[i]);
	}

	InitScreenQuad(&renderer->screenQuad, cmdBuffer);

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
			renderer->layers[0].renderTarget,
			1, &spriteVertexLayout
		);

		CreateBlendStateAlpha(&pipelineInfo.colorTargets[0].blend_state);

		renderer->spritePipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	renderer->quadShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/screenquad.frag.bin");

	{
		GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(
			SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			SDL_GPU_CULLMODE_BACK,
			renderer->quadShader,
			nullptr,
			1, &renderer->screenQuad.vertexBuffer->layout
		);

		CreateBlendStateAlpha(&pipelineInfo.colorTargets[0].blend_state);

		renderer->quadPipeline = CreateGraphicsPipeline(&pipelineInfo);
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

static void DestroyLayer(Renderer2DLayer* layer)
{
	DestroyRenderTarget(layer->renderTarget);

	SDL_ReleaseGPUBuffer(device, layer->spriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, layer->transferBuffer);
}

void DestroyRenderer2D(Renderer2D* renderer)
{
	SDL_ReleaseGPUSampler(device, renderer->sampler);
	SDL_ReleaseGPUTexture(device, renderer->emptyTexture);

	DestroyGraphicsPipeline(renderer->quadPipeline);
	DestroyShader(renderer->quadShader);

	DestroyGraphicsPipeline(renderer->spritePipeline);
	DestroyShader(renderer->spriteShader);

	DestroyScreenQuad(&renderer->screenQuad);

	for (int i = 0; i < renderer->numLayers; i++)
	{
		DestroyLayer(&renderer->layers[i]);
	}
}

static void ResizeLayer(Renderer2DLayer* layer, int width, int height)
{
	ResizeRenderTarget(layer->renderTarget, width, height);
}

void ResizeRenderer2D(Renderer2D* renderer, int width, int height)
{
	for (int i = 0; i < renderer->numLayers; i++)
	{
		ResizeLayer(&renderer->layers[i], width, height);
	}
}

void SetRenderer2DCamera(Renderer2D* renderer, int layerID, mat4 projectionView)
{
	SDL_assert(layerID < renderer->numLayers);

	Renderer2DLayer* layer = &renderer->layers[layerID];
	layer->vertexUniforms.projectionView = projectionView;
}

void BeginRenderer2D(Renderer2D* renderer)
{
	for (int i = 0; i < renderer->numLayers; i++)
	{
		renderer->layers[i].spriteDataPtr = (SpriteData*)SDL_MapGPUTransferBuffer(device, renderer->layers[i].transferBuffer, true);
		renderer->layers[i].numSprites = 0;
		renderer->layers[i].numTextures = 0;
	}
}

static int GetTextureID(Renderer2DLayer* layer, Renderer2D* renderer, Texture* texture)
{
	if (!texture)
		return -1;

	SDL_assert(layer->numTextures < MAX_RENDERER2D_LAYER_TEXTURES);

	int id = layer->numTextures++;
	layer->textures[id] = texture;
	return id;
}

void DrawSprite(Renderer2D* renderer, int layerID, const SpriteDrawData* drawData)
{
	SDL_assert(layerID < renderer->numLayers);

	Renderer2DLayer* layer = &renderer->layers[layerID];
	SDL_assert(layer->numSprites < layer->info.maxSprites);

	SpriteData* spriteData = &layer->spriteDataPtr[layer->numSprites++];

	spriteData->position = drawData->position;
	spriteData->rotation = drawData->rotation;
	spriteData->size = drawData->size;
	spriteData->rect = drawData->rect;
	spriteData->color = drawData->color;
	spriteData->textureID = GetTextureID(layer, renderer, drawData->texture);
}

static void RenderLayer(Renderer2DLayer* layer, Renderer2D* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (layer->numSprites == 0)
		return;

	SDL_GPURenderPass* renderPass = BindRenderTarget(layer->renderTarget, 0, cmdBuffer);

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->spritePipeline->pipeline);

	SDL_BindGPUVertexStorageBuffers(renderPass, 0, &layer->spriteDataBuffer, 1);

	SDL_PushGPUVertexUniformData(cmdBuffer, 0, &layer->vertexUniforms, sizeof(layer->vertexUniforms));

	SDL_GPUTextureSamplerBinding textureBindings[MAX_RENDERER2D_LAYER_TEXTURES] = {};
	for (int i = 0; i < MAX_RENDERER2D_LAYER_TEXTURES; i++)
	{
		textureBindings[i].texture = i < layer->numTextures ? layer->textures[i]->handle : renderer->emptyTexture;
		textureBindings[i].sampler = renderer->sampler;
	}
	SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, MAX_RENDERER2D_LAYER_TEXTURES);

	SDL_DrawGPUPrimitives(renderPass, layer->numSprites * 6, 1, 0, 0);

	SDL_EndGPURenderPass(renderPass);
}

void EndRenderer2D(Renderer2D* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	for (int i = 0; i < renderer->numLayers; i++)
	{
		Renderer2DLayer* layer = &renderer->layers[i];

		if (layer->numSprites == 0)
			continue;

		SDL_UnmapGPUTransferBuffer(device, layer->transferBuffer);
		layer->spriteDataPtr = nullptr;

		SDL_GPUTransferBufferLocation location = {};
		location.transfer_buffer = layer->transferBuffer;
		location.offset = 0;

		SDL_GPUBufferRegion region = {};
		region.buffer = layer->spriteDataBuffer;
		region.size = layer->numSprites * sizeof(SpriteData);
		region.offset = 0;

		SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
	}

	SDL_EndGPUCopyPass(copyPass);

	for (int i = 0; i < renderer->numLayers; i++)
	{
		RenderLayer(&renderer->layers[i], renderer, cmdBuffer);
	}

	SDL_GPUColorTargetInfo colorTargetInfo = {};
	colorTargetInfo.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	colorTargetInfo.texture = swapchain;

	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTargetInfo, 1, nullptr);

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->quadPipeline->pipeline);

	for (int i = 0; i < renderer->numLayers; i++)
	{
		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 1, renderer->layers[i].renderTarget->colorAttachments, &renderer->sampler, cmdBuffer);
	}

	SDL_EndGPURenderPass(renderPass);
}
