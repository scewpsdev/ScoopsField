#include "Renderer2D.h"

#include "math/Vector.h"
#include "math/Matrix.h"

#include <SDL3/SDL.h>



extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern SDL_GPUTexture* swapchain;


static Renderer2DLayer CreateLayer(Renderer2DLayerInfo* layerInfo)
{
	Renderer2DLayer layer = {};

	layer.info = *layerInfo;

	ColorAttachmentInfo colorAttachment = {};
	colorAttachment.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
	colorAttachment.loadOp = SDL_GPU_LOADOP_CLEAR;
	colorAttachment.storeOp = SDL_GPU_STOREOP_STORE;
	colorAttachment.clearColor = vec4(0.0f);
	layer.renderTarget = CreateRenderTarget(layerInfo->width, layerInfo->height, 1, &colorAttachment, nullptr);

	SDL_GPUBufferCreateInfo spriteDataBufferInfo = {};
	spriteDataBufferInfo.size = layerInfo->maxSprites * sizeof(SpriteData);
	spriteDataBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;

	layer.spriteDataBuffer = SDL_CreateGPUBuffer(device, &spriteDataBufferInfo);

	SDL_GPUTransferBufferCreateInfo transferBufferInfo = {};
	transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBufferInfo.size = layerInfo->maxSprites * sizeof(SpriteData);

	layer.transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

	layer.vertexUniforms.projectionView = Matrix::Identity;

	return layer;
}

void InitRenderer2D(Renderer2D* renderer, int numLayers, Renderer2DLayerInfo* layerInfo, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->numLayers = numLayers;
	renderer->layers = (Renderer2DLayer*)SDL_malloc(numLayers * sizeof(Renderer2DLayer));

	for (int i = 0; i < numLayers; i++)
		renderer->layers[i] = CreateLayer(&layerInfo[i]);

	renderer->triangleShader = LoadGraphicsShader("res/shaders/sprite.vs.glsl.bin", "res/shaders/sprite.fs.glsl.bin");
	renderer->texture = LoadTexture("res/textures/ravioli_atlas.png.bin", cmdBuffer);

	{
		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.vertex_shader = renderer->triangleShader->vertex;
		pipelineInfo.fragment_shader = renderer->triangleShader->fragment;
		pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

		SDL_GPUColorTargetDescription targetDescriptions[1];
		targetDescriptions[0] = {};
		targetDescriptions[0].format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
		targetDescriptions[0].blend_state.enable_blend = true;
		targetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
		targetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
		targetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
		targetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		targetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
		targetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

		pipelineInfo.target_info.num_color_targets = 1;
		pipelineInfo.target_info.color_target_descriptions = targetDescriptions;

		renderer->pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
	}

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->sampler = SDL_CreateGPUSampler(device, &samplerInfo);

	InitScreenQuad(&renderer->screenQuad, cmdBuffer);

	renderer->quadShader = LoadGraphicsShader("res/shaders/screenquad.vs.glsl.bin", "res/shaders/screenquad.fs.glsl.bin");

	{
		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.vertex_shader = renderer->quadShader->vertex;
		pipelineInfo.fragment_shader = renderer->quadShader->fragment;
		pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

		SDL_GPUColorTargetDescription targetDescriptions[1];
		targetDescriptions[0] = {};
		targetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);
		targetDescriptions[0].blend_state.enable_blend = true;
		targetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
		targetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
		targetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
		targetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		targetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
		targetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

		pipelineInfo.target_info.num_color_targets = 1;
		pipelineInfo.target_info.color_target_descriptions = targetDescriptions;

		SDL_GPUVertexAttribute attributes[1];
		attributes[0].buffer_slot = 0;
		attributes[0].location = 0;
		attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
		attributes[0].offset = 0;

		pipelineInfo.vertex_input_state.num_vertex_attributes = 1;
		pipelineInfo.vertex_input_state.vertex_attributes = attributes;

		SDL_GPUVertexBufferDescription bufferDescriptions[1];
		bufferDescriptions[0].slot = 0;
		bufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		bufferDescriptions[0].instance_step_rate = 0;
		bufferDescriptions[0].pitch = sizeof(vec3);

		pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
		pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescriptions;

		renderer->quadPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
	}
}

static void DestroyLayer(Renderer2DLayer* layer)
{
	DestroyRenderTarget(layer->renderTarget);

	SDL_ReleaseGPUBuffer(device, layer->spriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, layer->transferBuffer);
}

void DestroyRenderer2D(Renderer2D* renderer)
{
	DestroyShader(renderer->quadShader);

	SDL_ReleaseGPUGraphicsPipeline(device, renderer->quadPipeline);

	DestroyScreenQuad(&renderer->screenQuad);

	DestroyTexture(renderer->texture);
	DestroyShader(renderer->triangleShader);

	for (int i = 0; i < renderer->numLayers; i++)
	{
		DestroyLayer(&renderer->layers[i]);
	}
	SDL_free(renderer->layers);

	SDL_ReleaseGPUSampler(device, renderer->sampler);
	SDL_ReleaseGPUGraphicsPipeline(device, renderer->pipeline);
}

void Renderer2DSetProjectionView(Renderer2D* renderer, int layerID, Matrix projectionView)
{
	SDL_assert(layerID < renderer->numLayers);

	Renderer2DLayer* layer = &renderer->layers[layerID];
	layer->vertexUniforms.projectionView = projectionView;
}

void Renderer2DBegin(Renderer2D* renderer)
{
	for (int i = 0; i < renderer->numLayers; i++)
	{
		renderer->layers[i].spriteDataPtr = (SpriteData*)SDL_MapGPUTransferBuffer(device, renderer->layers[i].transferBuffer, true);
		renderer->layers[i].numSprites = 0;
	}
}

void Renderer2DDrawSprite(Renderer2D* renderer, int layerID, SpriteDrawData drawData)
{
	SDL_assert(layerID < renderer->numLayers);

	Renderer2DLayer* layer = &renderer->layers[layerID];
	SDL_assert(layer->numSprites < layer->info.maxSprites);

	SpriteData* spriteData = &layer->spriteDataPtr[layer->numSprites++];

	spriteData->position = drawData.position;
	spriteData->rotation = drawData.rotation;
	spriteData->size = drawData.size;
	spriteData->rect = drawData.rect;
	spriteData->color = drawData.color;
}

static void RenderLayer(Renderer2DLayer* layer, Renderer2D* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (layer->numSprites == 0)
		return;

	SDL_GPURenderPass* renderPass = BindRenderTarget(layer->renderTarget, cmdBuffer);

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->pipeline);

	SDL_BindGPUVertexStorageBuffers(renderPass, 0, &layer->spriteDataBuffer, 1);

	SDL_PushGPUVertexUniformData(cmdBuffer, 0, &layer->vertexUniforms, sizeof(layer->vertexUniforms));

	SDL_GPUTextureSamplerBinding binding = {};
	binding.texture = renderer->texture->handle;
	binding.sampler = renderer->sampler;

	SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

	SDL_DrawGPUPrimitives(renderPass, layer->numSprites * 6, 1, 0, 0);

	SDL_EndGPURenderPass(renderPass);
}

void Renderer2DEnd(Renderer2D* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	for (int i = 0; i < renderer->numLayers; i++)
	{
		if (renderer->layers[i].numSprites == 0)
			continue;

		SDL_UnmapGPUTransferBuffer(device, renderer->layers[i].transferBuffer);
		renderer->layers[i].spriteDataPtr = nullptr;

		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		SDL_GPUTransferBufferLocation location = {};
		location.transfer_buffer = renderer->layers[i].transferBuffer;
		location.offset = 0;

		SDL_GPUBufferRegion region = {};
		region.buffer = renderer->layers[i].spriteDataBuffer;
		region.size = renderer->layers[i].numSprites * sizeof(SpriteData);
		region.offset = 0;

		SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
		SDL_EndGPUCopyPass(copyPass);
	}

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

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->quadPipeline);

	for (int i = 0; i < renderer->numLayers; i++)
	{
		RenderScreenQuad(&renderer->screenQuad, renderPass, 1, renderer->layers[i].renderTarget->colorAttachments, renderer->sampler, cmdBuffer);
	}

	SDL_EndGPURenderPass(renderPass);
}
