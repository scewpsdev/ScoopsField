#include "Renderer.h"

#include "Application.h"


static const vec3 cubeVertices[8] = {
	vec3(-1), vec3(1, -1, -1), vec3(-1, -1, 1), vec3(1, -1, 1),
	vec3(-1, 1, -1), vec3(1, 1, -1), vec3(-1, 1, 1), vec3(1, 1, 1),
};
static const uint16_t cubeIndices[36] = {
	0, 2, 6, 6, 4, 0,
	1, 5, 7, 7, 3, 1,
	0, 1, 3, 3, 2, 0,
	4, 6, 7, 7, 5, 4,
	0, 4, 5, 5, 1, 0,
	2, 3, 7, 7, 6, 2,
};


extern GameMemory* memory;
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
#define GBUFFER_COLOR_ATTACHMENTS 2
	ColorAttachmentInfo colorAttachments[GBUFFER_COLOR_ATTACHMENTS];
	// normal
	colorAttachments[0] = {};
	colorAttachments[0].format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	colorAttachments[0].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[0].storeOp = SDL_GPU_STOREOP_STORE;
	// color
	colorAttachments[1] = {};
	colorAttachments[1].format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	colorAttachments[1].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[1].storeOp = SDL_GPU_STOREOP_STORE;

	DepthAttachmentInfo depthAttachment = {};
	depthAttachment.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depthAttachment.loadOp = SDL_GPU_LOADOP_CLEAR;
	depthAttachment.storeOp = SDL_GPU_STOREOP_STORE;
	depthAttachment.clearDepth = 1;

	return CreateRenderTarget(width, height, GBUFFER_COLOR_ATTACHMENTS, colorAttachments, &depthAttachment);
}

void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->width = width;
	renderer->height = height;

	InitList(&renderer->meshes);
	InitList(&renderer->pointLights);

	renderer->depthTexture = CreateDepthTarget(width, height);
	renderer->gbuffer = CreateGBuffer(width, height);

	renderer->defaultShader = LoadGraphicsShader("res/shaders/mesh.vert.bin", "res/shaders/mesh.frag.bin");

	{
		GraphicsPipelineInfo pipelineInfo = {};

		pipelineInfo.shader = renderer->defaultShader;
		pipelineInfo.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		pipelineInfo.cullMode = SDL_GPU_CULLMODE_BACK;

		pipelineInfo.numColorTargets = renderer->gbuffer->numColorAttachments;
		for (int i = 0; i < pipelineInfo.numColorTargets; i++)
		{
			pipelineInfo.colorTargets[i].format = renderer->gbuffer->colorAttachmentInfos[i].format;
			pipelineInfo.colorTargets[i].blend_state.enable_blend = false;
		}

		pipelineInfo.hasDepthTarget = true;
		pipelineInfo.depthFormat = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;

		pipelineInfo.numAttributes = 2;
		pipelineInfo.attributes[0] = {};
		pipelineInfo.attributes[0].buffer_slot = 0;
		pipelineInfo.attributes[0].location = 0;
		pipelineInfo.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
		pipelineInfo.attributes[0].offset = 0;
		pipelineInfo.attributes[1] = {};
		pipelineInfo.attributes[1].buffer_slot = 1;
		pipelineInfo.attributes[1].location = 1;
		pipelineInfo.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
		pipelineInfo.attributes[1].offset = 0;

		pipelineInfo.numVertexBuffers = 2;
		pipelineInfo.bufferDescriptions[0].slot = 0;
		pipelineInfo.bufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		pipelineInfo.bufferDescriptions[0].instance_step_rate = 0;
		pipelineInfo.bufferDescriptions[0].pitch = GetVertexFormatSize(pipelineInfo.attributes[0].format);
		pipelineInfo.bufferDescriptions[1].slot = 1;
		pipelineInfo.bufferDescriptions[1].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		pipelineInfo.bufferDescriptions[1].instance_step_rate = 0;
		pipelineInfo.bufferDescriptions[1].pitch = GetVertexFormatSize(pipelineInfo.attributes[1].format);

		renderer->geometryPipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	renderer->directionalLightShader = LoadGraphicsShader("res/shaders/lighting/directional_light.vert.bin", "res/shaders/lighting/directional_light.frag.bin");

	{
		GraphicsPipelineInfo pipelineInfo = {};

		pipelineInfo.shader = renderer->directionalLightShader;
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

		renderer->directionalLightPipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	renderer->pointLightShader = LoadGraphicsShader("res/shaders/lighting/point_light.vert.bin", "res/shaders/lighting/point_light.frag.bin");

	{
		GraphicsPipelineInfo pipelineInfo = {};

		pipelineInfo.shader = renderer->pointLightShader;
		pipelineInfo.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		pipelineInfo.cullMode = SDL_GPU_CULLMODE_BACK;

		pipelineInfo.numColorTargets = 1;
		pipelineInfo.colorTargets[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);
		pipelineInfo.colorTargets[0].blend_state.enable_blend = false;

		pipelineInfo.depthWrite = false;

		pipelineInfo.numAttributes = 3;
		pipelineInfo.attributes[0] = {};
		pipelineInfo.attributes[0].buffer_slot = 0;
		pipelineInfo.attributes[0].offset = 0;
		pipelineInfo.attributes[0].location = 0;
		pipelineInfo.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
		pipelineInfo.attributes[1] = {};
		pipelineInfo.attributes[1].buffer_slot = 1;
		pipelineInfo.attributes[1].offset = 0;
		pipelineInfo.attributes[1].location = 1;
		pipelineInfo.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
		pipelineInfo.attributes[2] = {};
		pipelineInfo.attributes[2].buffer_slot = 1;
		pipelineInfo.attributes[2].offset = sizeof(vec3);
		pipelineInfo.attributes[2].location = 2;
		pipelineInfo.attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

		pipelineInfo.numVertexBuffers = 2;
		pipelineInfo.bufferDescriptions[0].slot = 0;
		pipelineInfo.bufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		pipelineInfo.bufferDescriptions[0].instance_step_rate = 0;
		pipelineInfo.bufferDescriptions[0].pitch = GetVertexFormatSize(pipelineInfo.attributes[0].format);
		pipelineInfo.bufferDescriptions[1].slot = 1;
		pipelineInfo.bufferDescriptions[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
		pipelineInfo.bufferDescriptions[1].instance_step_rate = 0;
		pipelineInfo.bufferDescriptions[1].pitch = sizeof(LightDrawData);

		renderer->pointLightPipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	VertexBufferLayout cubeLayout = {};
	cubeLayout.numAttributes = 1;
	cubeLayout.attributes[0].location = 0;
	cubeLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	cubeLayout.perInstance = false;

	renderer->cubeVertexBuffer = CreateVertexBuffer(8, &cubeLayout, 0);
	UpdateVertexBuffer(renderer->cubeVertexBuffer, 0, (const uint8_t*)cubeVertices, sizeof(cubeVertices), copyPass);

	renderer->cubeIndexBuffer = CreateIndexBuffer(36, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	UpdateIndexBuffer(renderer->cubeIndexBuffer, 0, (const uint8_t*)cubeIndices, sizeof(cubeIndices), copyPass);

	VertexBufferLayout pointLightInstanceLayout = {};
	pointLightInstanceLayout.numAttributes = 2;
	pointLightInstanceLayout.attributes[0].location = 1;
	pointLightInstanceLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	pointLightInstanceLayout.attributes[1].location = 2;
	pointLightInstanceLayout.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	pointLightInstanceLayout.perInstance = true;

	renderer->pointLightInstanceBuffer = CreateVertexBuffer(MAX_POINT_LIGHT_DRAWS, &pointLightInstanceLayout, 0);

	renderer->pointLightInstanceTransferBuffer = CreateTransferBuffer(MAX_POINT_LIGHT_DRAWS * sizeof(LightDrawData), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	MapTransferBuffer(renderer->pointLightInstanceTransferBuffer);

	InitScreenQuad(&renderer->screenQuad, copyPass);

	SDL_EndGPUCopyPass(copyPass);

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->defaultSampler = SDL_CreateGPUSampler(device, &samplerInfo);
}

void DestroyRenderer(Renderer* renderer)
{
	SDL_ReleaseGPUSampler(device, renderer->defaultSampler);

	DestroyScreenQuad(&renderer->screenQuad);

	UnmapTransferBuffer(renderer->pointLightInstanceTransferBuffer);
	DestroyTransferBuffer(renderer->pointLightInstanceTransferBuffer);

	DestroyVertexBuffer(renderer->pointLightInstanceBuffer);

	DestroyGraphicsPipeline(renderer->directionalLightPipeline);
	DestroyShader(renderer->directionalLightShader);

	DestroyGraphicsPipeline(renderer->pointLightPipeline);
	DestroyShader(renderer->pointLightShader);

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

void RenderMesh(Renderer* renderer, Mesh* mesh, mat4 transform)
{
	MeshDrawData data = {};
	data.mesh = mesh;
	data.transform = transform;
	renderer->meshes.add(data);
}

void RenderLight(Renderer* renderer, vec3 position, vec3 color)
{
	LightDrawData data = {};
	data.position = position;
	data.color = color;
	renderer->pointLights.add(data);
}

static void SubmitMesh(Mesh* mesh, const mat4& transform, const mat4& pv, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUBufferBinding vertexBindings[2];
	vertexBindings[0] = {};
	vertexBindings[0].buffer = mesh->positionBuffer->buffer;
	vertexBindings[0].offset = 0;
	vertexBindings[1] = {};
	vertexBindings[1].buffer = mesh->normalBuffer->buffer;
	vertexBindings[1].offset = 0;

	SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 2);

	SDL_GPUBufferBinding indexBinding = {};
	indexBinding.buffer = mesh->indexBuffer->buffer;
	indexBinding.offset = 0;

	SDL_BindGPUIndexBuffer(renderPass, &indexBinding, mesh->indexBuffer->elementSize);

	struct UniformData
	{
		mat4 projectionViewModel;
	};
	UniformData uniforms = {};
	uniforms.projectionViewModel = pv * transform;
	SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

	SDL_DrawGPUIndexedPrimitives(renderPass, mesh->indexCount, 1, 0, 0, 0);
}

void RendererShow(Renderer* renderer, mat4 projection, mat4 view, float near, float far, SDL_GPUCommandBuffer* cmdBuffer)
{
	mat4 pv = projection * view;

	vec4 frustumPlanes[6];
	GetFrustumPlanes(pv, frustumPlanes);

	// geometry pass
	{
		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->gbuffer, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->geometryPipeline->pipeline);

		for (int i = 0; i < renderer->meshes.size; i++)
		{
			SubmitMesh(renderer->meshes[i].mesh, renderer->meshes[i].transform, pv, renderPass, cmdBuffer);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	// lighting pass
	{
		SDL_GPUColorTargetInfo colorTarget = {};
		colorTarget.clear_color = { 0.4f, 0.4f, 1.0f, 1.0f };
		colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTarget.store_op = SDL_GPU_STOREOP_STORE;
		colorTarget.texture = swapchain;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTarget, 1, nullptr);

		// directional lights
		{
			SDL_BindGPUGraphicsPipeline(renderPass, renderer->directionalLightPipeline->pipeline);

			struct UniformData
			{
				vec4 cameraParams;
				vec4 lightDirection;
				vec4 lightColor;
			};
			UniformData uniforms = {};
			uniforms.cameraParams = vec4(near, far, 0, 0);
			uniforms.lightDirection = vec4(-vec3(1, 2, 0.5).normalized(), 0);
			uniforms.lightColor = vec4(1, 1, 1, 0);
			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTexture* gbufferTextures[MAX_COLOR_ATTACHMENTS + 1];
			for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
				gbufferTextures[i] = renderer->gbuffer->colorAttachments[i];
			gbufferTextures[renderer->gbuffer->numColorAttachments] = renderer->gbuffer->depthAttachment;

			RenderScreenQuad(&renderer->screenQuad, renderPass, renderer->gbuffer->numColorAttachments + 1, gbufferTextures, renderer->defaultSampler, cmdBuffer);
		}

		// point lights
		{
			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);
			SDL_memcpy(renderer->pointLightInstanceTransferBuffer->mapped, renderer->pointLights.buffer, renderer->pointLights.size * sizeof(LightDrawData));
			UpdateVertexBuffer(renderer->pointLightInstanceBuffer, 0, renderer->pointLights.size * sizeof(LightDrawData), renderer->pointLightInstanceTransferBuffer->buffer, copyPass);
			SDL_EndGPUCopyPass(copyPass); copyPass = nullptr;

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->pointLightPipeline->pipeline);

			SDL_GPUBufferBinding vertexBindings[2];
			vertexBindings[0] = {};
			vertexBindings[0].buffer = renderer->cubeVertexBuffer->buffer;
			vertexBindings[0].offset = 0;
			vertexBindings[1] = {};
			vertexBindings[1].buffer = renderer->pointLightInstanceBuffer->buffer;
			vertexBindings[1].offset = 0;

			SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 2);

			SDL_GPUBufferBinding indexBinding = {};
			indexBinding.buffer = renderer->cubeIndexBuffer->buffer;
			indexBinding.offset = 0;

			SDL_BindGPUIndexBuffer(renderPass, &indexBinding, renderer->cubeIndexBuffer->elementSize);

			SDL_GPUTextureSamplerBinding textureBindings[MAX_COLOR_ATTACHMENTS + 1];
			for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			{
				textureBindings[i].texture = renderer->gbuffer->colorAttachments[i];
				textureBindings[i].sampler = renderer->defaultSampler;
			}
			textureBindings[renderer->gbuffer->numColorAttachments].texture = renderer->gbuffer->depthAttachment;
			textureBindings[renderer->gbuffer->numColorAttachments].sampler = renderer->defaultSampler;

			SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, renderer->gbuffer->numColorAttachments + 1);

			SDL_DrawGPUPrimitives(renderPass, renderer->cubeIndexBuffer->numIndices, renderer->pointLightInstanceBuffer->numVertices, 0, 0);
		}

		SDL_EndGPURenderPass(renderPass);
	}
}
