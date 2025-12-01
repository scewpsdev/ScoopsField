#include "Renderer.h"

#include "Application.h"


extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern SDL_GPUTexture* swapchain;


static SDL_GPUTexture* CreateDepthTarget(int width, int height)
{
	SDL_GPUTextureCreateInfo depthTextureInfo = {};
	depthTextureInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depthTextureInfo.width = width;
	depthTextureInfo.height = height;
	depthTextureInfo.layer_count_or_depth = 1;
	depthTextureInfo.num_levels = 1;
	depthTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	depthTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	return SDL_CreateGPUTexture(device, &depthTextureInfo);
}

static RenderTarget* CreateGBuffer(int width, int height)
{
#define GBUFFER_COLOR_ATTACHMENTS 3
	ColorAttachmentInfo colorAttachments[GBUFFER_COLOR_ATTACHMENTS];
	// position
	colorAttachments[0] = {};
	colorAttachments[0].format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
	colorAttachments[0].loadOp = SDL_GPU_LOADOP_CLEAR;
	colorAttachments[0].storeOp = SDL_GPU_STOREOP_STORE;
	colorAttachments[0].clearColor = vec4(0.0f);
	// normal
	colorAttachments[1] = {};
	colorAttachments[1].format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	colorAttachments[1].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[1].storeOp = SDL_GPU_STOREOP_STORE;
	colorAttachments[1].clearColor = vec4(0.0f);
	// color
	colorAttachments[2] = {};
	colorAttachments[2].format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	colorAttachments[2].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[2].storeOp = SDL_GPU_STOREOP_STORE;

	DepthAttachmentInfo depthAttachment = {};
	depthAttachment.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depthAttachment.loadOp = SDL_GPU_LOADOP_CLEAR;
	depthAttachment.storeOp = SDL_GPU_STOREOP_STORE;
	depthAttachment.clearDepth = 1;

	return CreateRenderTarget(width, height, GBUFFER_COLOR_ATTACHMENTS, colorAttachments, &depthAttachment);
}

void InitRenderer(Renderer* renderer, int width, int height)
{
	renderer->width = width;
	renderer->height = height;

	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(device);

	renderer->depthTexture = CreateDepthTarget(width, height);
	renderer->gbuffer = CreateGBuffer(width, height);

	renderer->lightingShader = LoadGraphicsShader("res/shaders/lighting.vert.bin", "res/shaders/lighting.frag.bin");

	{
		GraphicsPipelineInfo pipelineInfo = {};

		pipelineInfo.shader = renderer->lightingShader;
		pipelineInfo.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		pipelineInfo.cullMode = SDL_GPU_CULLMODE_BACK;

		pipelineInfo.numColorTargets = 1;
		pipelineInfo.colorTargets[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);
		pipelineInfo.colorTargets[0].blend_state.enable_blend = false;

		pipelineInfo.numAttributes = 1;
		pipelineInfo.attributes[0] = {};
		pipelineInfo.attributes[0].buffer_slot = 0;
		pipelineInfo.attributes[0].location = 0;
		pipelineInfo.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
		pipelineInfo.attributes[0].offset = 0;

		pipelineInfo.numVertexBuffers = 1;
		pipelineInfo.bufferDescriptions[0].slot = 0;
		pipelineInfo.bufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		pipelineInfo.bufferDescriptions[0].instance_step_rate = 0;
		pipelineInfo.bufferDescriptions[0].pitch = GetVertexFormatSize(pipelineInfo.attributes[0].format);

		renderer->lightingPipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	InitScreenQuad(&renderer->screenQuad, cmdBuffer);

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->defaultSampler = SDL_CreateGPUSampler(device, &samplerInfo);

	SDL_SubmitGPUCommandBuffer(cmdBuffer);
}

void DestroyRenderer(Renderer* renderer)
{
	SDL_ReleaseGPUSampler(device, renderer->defaultSampler);

	DestroyScreenQuad(&renderer->screenQuad);

	DestroyGraphicsPipeline(renderer->lightingPipeline);
	DestroyShader(renderer->lightingShader);

	DestroyRenderTarget(renderer->gbuffer);
	SDL_ReleaseGPUTexture(device, renderer->depthTexture);
}

void ResizeRenderer(Renderer* renderer, int width, int height)
{
	renderer->width = width;
	renderer->height = height;

	if (renderer->depthTexture)
		SDL_ReleaseGPUTexture(device, renderer->depthTexture);
	renderer->depthTexture = CreateDepthTarget(width, height);

	if (renderer->gbuffer)
		DestroyRenderTarget(renderer->gbuffer);
	renderer->gbuffer = CreateGBuffer(width, height);
}

void RenderScene(Renderer* renderer, GameState* game, SDL_GPUCommandBuffer* cmdBuffer)
{
	Matrix projection = Matrix::Perspective(90 * Deg2Rad, renderer->width / (float)renderer->height, 1, 8000);
	Matrix view = Matrix::Rotate(game->cameraRotation.conjugated()) * Matrix::Translate(-game->cameraPosition);
	Matrix pv = projection * view;

	vec4 frustumPlanes[6];
	GetFrustumPlanes(pv, frustumPlanes);

	// geometry pass
	{
		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->gbuffer, cmdBuffer);

		//

		SDL_EndGPURenderPass(renderPass);
	}

	// lighting pass
	{
		SDL_GPUColorTargetInfo colorTarget = {};
		colorTarget.clear_color = { 0.4f, 0.4f, 1.0f, 1.0f };
		colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTarget.store_op = SDL_GPU_STOREOP_STORE;
		colorTarget.texture = swapchain;

		SDL_GPUDepthStencilTargetInfo depthTarget = {};
		depthTarget.clear_depth = 1;
		depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
		depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;
		depthTarget.texture = renderer->depthTexture;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTarget, 1, &depthTarget);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->lightingPipeline->pipeline);

		SDL_GPUTexture* gbufferTextures[MAX_COLOR_ATTACHMENTS + 1];
		for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			gbufferTextures[i] = renderer->gbuffer->colorAttachments[i];
		gbufferTextures[renderer->gbuffer->numColorAttachments] = renderer->gbuffer->depthAttachment;

		RenderScreenQuad(&renderer->screenQuad, renderPass, renderer->gbuffer->numColorAttachments + 1, gbufferTextures, renderer->defaultSampler, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}
}
