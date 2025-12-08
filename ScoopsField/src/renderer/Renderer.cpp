#include "Renderer.h"

#include "Application.h"


struct LightInstanceData
{
	vec4 positionRadius;
	vec4 color;
};


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

static GraphicsPipeline* CreateGeometryPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->defaultShader, renderer->gbuffer, 2, renderer->meshLayout);
	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateAnimatedPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->animatedShader, renderer->gbuffer, 3, renderer->animatedLayout);
	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateCopyDepthPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->copyDepthShader, renderer->hdrTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	//pipelineInfo.numColorTargets = 0;
	//pipelineInfo.depthTest = false;
	//pipelineInfo.depthWrite = true;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateDirectionalLightPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->directionalLightShader, renderer->hdrTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	CreateBlendStateAddPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreatePointLightPipeline(Renderer* renderer)
{
	VertexBufferLayout bufferLayouts[2] = { renderer->cubeVertexBuffer->layout, renderer->pointLightInstanceBuffer->layout };
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_FRONT, renderer->pointLightShader, renderer->hdrTarget, 2, bufferLayouts);

	CreateBlendStateAddPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

	pipelineInfo.depthWrite = false;
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateTonemappingPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->tonemappingShader, nullptr, 1, &renderer->screenQuad.vertexBuffer->layout);
	return CreateGraphicsPipeline(&pipelineInfo);
}

void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->width = width;
	renderer->height = height;

	InitList(&renderer->meshes);
	InitList(&renderer->pointLights);

	renderer->depthTexture = CreateDepthTarget(width, height);
	renderer->gbuffer = CreateGBuffer(width, height);

	ColorAttachmentInfo hdrTargetInfo = {};
	hdrTargetInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	hdrTargetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrTargetInfo.storeOp = SDL_GPU_STOREOP_STORE;
	hdrTargetInfo.clearColor = { 0, 0, 0, 0 };

	DepthAttachmentInfo hdrDepthInfo = {};
	hdrDepthInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	hdrDepthInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrDepthInfo.storeOp = SDL_GPU_STOREOP_DONT_CARE;
	hdrDepthInfo.clearDepth = 1;

	renderer->hdrTarget = CreateRenderTarget(width, height, 1, &hdrTargetInfo, &hdrDepthInfo);

	renderer->meshLayout[0].numAttributes = 1;
	renderer->meshLayout[0].attributes[0].location = 0;
	renderer->meshLayout[0].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	renderer->meshLayout[1].numAttributes = 1;
	renderer->meshLayout[1].attributes[0].location = 1;
	renderer->meshLayout[1].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

	renderer->animatedLayout[0].numAttributes = 1;
	renderer->animatedLayout[0].attributes[0].location = 0;
	renderer->animatedLayout[0].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	renderer->animatedLayout[1].numAttributes = 1;
	renderer->animatedLayout[1].attributes[0].location = 1;
	renderer->animatedLayout[1].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	renderer->animatedLayout[2].numAttributes = 2;
	renderer->animatedLayout[2].attributes[0].location = 2;
	renderer->animatedLayout[2].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	renderer->animatedLayout[2].attributes[1].location = 3;
	renderer->animatedLayout[2].attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;

	InitScreenQuad(&renderer->screenQuad, cmdBuffer);

	VertexBufferLayout cubeLayout = {};
	cubeLayout.numAttributes = 1;
	cubeLayout.attributes[0].location = 0;
	cubeLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	cubeLayout.perInstance = false;

	renderer->cubeVertexBuffer = CreateVertexBuffer(8, &cubeLayout, 0);
	UpdateVertexBuffer(renderer->cubeVertexBuffer, 0, (const uint8_t*)cubeVertices, sizeof(cubeVertices), cmdBuffer);

	renderer->cubeIndexBuffer = CreateIndexBuffer(36, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	UpdateIndexBuffer(renderer->cubeIndexBuffer, 0, (const uint8_t*)cubeIndices, sizeof(cubeIndices), cmdBuffer);

	VertexBufferLayout pointLightInstanceLayout = {};
	pointLightInstanceLayout.numAttributes = 2;
	pointLightInstanceLayout.attributes[0].location = 1;
	pointLightInstanceLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	pointLightInstanceLayout.attributes[1].location = 2;
	pointLightInstanceLayout.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	pointLightInstanceLayout.perInstance = true;

	renderer->pointLightInstanceBuffer = CreateVertexBuffer(MAX_POINT_LIGHT_DRAWS, &pointLightInstanceLayout, 0);

	renderer->pointLightInstanceTransferBuffer = CreateTransferBuffer(MAX_POINT_LIGHT_DRAWS * sizeof(LightInstanceData), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	MapTransferBuffer(renderer->pointLightInstanceTransferBuffer);

	renderer->defaultShader = LoadGraphicsShader("res/shaders/mesh.vert.bin", "res/shaders/mesh.frag.bin");
	renderer->animatedShader = LoadGraphicsShader("res/shaders/mesh_animated.vert.bin", "res/shaders/mesh.frag.bin");
	renderer->copyDepthShader = LoadGraphicsShader("res/shaders/copy_depth.vert.bin", "res/shaders/copy_depth.frag.bin");
	renderer->directionalLightShader = LoadGraphicsShader("res/shaders/lighting/directional_light.vert.bin", "res/shaders/lighting/directional_light.frag.bin");
	renderer->pointLightShader = LoadGraphicsShader("res/shaders/lighting/point_light.vert.bin", "res/shaders/lighting/point_light.frag.bin");
	renderer->tonemappingShader = LoadGraphicsShader("res/shaders/tonemapping.vert.bin", "res/shaders/tonemapping.frag.bin");

	renderer->geometryPipeline = CreateGeometryPipeline(renderer);
	renderer->animatedPipeline = CreateAnimatedPipeline(renderer);
	renderer->copyDepthPipeline = CreateCopyDepthPipeline(renderer);
	renderer->directionalLightPipeline = CreateDirectionalLightPipeline(renderer);
	renderer->pointLightPipeline = CreatePointLightPipeline(renderer);
	renderer->tonemappingPipeline = CreateTonemappingPipeline(renderer);

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->defaultSampler = SDL_CreateGPUSampler(device, &samplerInfo);

	SDL_GPUBufferCreateInfo emptyBufferInfo = {};
	emptyBufferInfo.size = 4;
	emptyBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	renderer->emptyBuffer = SDL_CreateGPUBuffer(device, &emptyBufferInfo);
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

void RenderMesh(Renderer* renderer, Mesh* mesh, SkeletonState* skeleton, mat4 transform)
{
	MeshDrawData data = {};
	data.mesh = mesh;
	data.transform = transform;
	data.skeleton = skeleton;

	if (skeleton)
		renderer->animatedMeshes.add(data);
	else
		renderer->meshes.add(data);
}

void RenderModel(Renderer* renderer, Model* model, AnimationState* animation, mat4 transform)
{
	for (int i = 0; i < model->numMeshes; i++)
	{
		RenderMesh(renderer, &model->meshes[i], &animation->skeletons[i], transform);
	}
}

void RenderLight(Renderer* renderer, vec3 position, vec3 color)
{
	LightDrawData data = {};
	data.position = position;
	data.color = color;
	renderer->pointLights.add(data);
}

static void SubmitMesh(Renderer* renderer, Mesh* mesh, SkeletonState* skeleton, const mat4& transform, const mat4& pv, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (skeleton)
	{
		SDL_GPUBufferBinding vertexBindings[3] = {};
		vertexBindings[0].buffer = mesh->positionBuffer ? mesh->positionBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[0].offset = 0;
		vertexBindings[1].buffer = mesh->normalBuffer ? mesh->normalBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[1].offset = 0;
		vertexBindings[2].buffer = mesh->weightsBuffer ? mesh->weightsBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[2].offset = 0;

		SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 3);
	}
	else
	{
		SDL_GPUBufferBinding vertexBindings[2] = {};
		vertexBindings[0].buffer = mesh->positionBuffer ? mesh->positionBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[0].offset = 0;
		vertexBindings[1].buffer = mesh->normalBuffer ? mesh->normalBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[1].offset = 0;

		SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 2);
	}

	SDL_GPUBufferBinding indexBinding = {};
	indexBinding.buffer = mesh->indexBuffer->buffer;
	indexBinding.offset = 0;

	SDL_BindGPUIndexBuffer(renderPass, &indexBinding, mesh->indexBuffer->elementSize);

	if (skeleton)
	{
		struct UniformData
		{
			mat4 projectionViewModel;
			mat4 model;
			mat4 boneTransforms[MAX_BONES];
		};
		UniformData uniforms = {};
		uniforms.projectionViewModel = pv * transform;
		uniforms.model = transform;

		SDL_memcpy(uniforms.boneTransforms, skeleton->boneTransforms, skeleton->numBones * sizeof(mat4));

		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));
	}
	else
	{
		struct UniformData
		{
			mat4 projectionViewModel;
			mat4 model;
		};
		UniformData uniforms = {};
		uniforms.projectionViewModel = pv * transform;
		uniforms.model = transform;

		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));
	}

	SDL_DrawGPUIndexedPrimitives(renderPass, mesh->indexCount, 1, 0, 0, 0);
}

static float CalculateLightRadius(vec3 color)
{
	return 3;
}

// TODO
// render meshes front to back
// frustum culling
// stencil for light volumes
// mesh occlusion culling
// light occlusion culling
// mesh instancing

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
			SubmitMesh(renderer, renderer->meshes[i].mesh, nullptr, renderer->meshes[i].transform, pv, renderPass, cmdBuffer);
		}

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->animatedPipeline->pipeline);

		for (int i = 0; i < renderer->animatedMeshes.size; i++)
		{
			Mesh* mesh = renderer->animatedMeshes[i].mesh;
			SkeletonState* skeleton = renderer->animatedMeshes[i].skeleton;
			const mat4& transform = renderer->animatedMeshes[i].transform;
			SubmitMesh(renderer, mesh, skeleton, transform, pv, renderPass, cmdBuffer);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	// lighting pass
	{
		// update point light data
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);
		LightInstanceData* instanceData = (LightInstanceData*)renderer->pointLightInstanceTransferBuffer->mapped;
		for (int i = 0; i < renderer->pointLights.size; i++)
		{
			vec3 position = renderer->pointLights[i].position;
			vec3 color = renderer->pointLights[i].color;
			instanceData[i].positionRadius = vec4(position, CalculateLightRadius(color));
			instanceData[i].color = vec4(color, 0);
		}
		UpdateVertexBuffer(renderer->pointLightInstanceBuffer, 0, renderer->pointLights.size * sizeof(LightInstanceData), renderer->pointLightInstanceTransferBuffer->buffer, copyPass);
		SDL_EndGPUCopyPass(copyPass); copyPass = nullptr;


		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->hdrTarget, cmdBuffer);

		// copy depth
		{
			SDL_BindGPUGraphicsPipeline(renderPass, renderer->copyDepthPipeline->pipeline);

			RenderScreenQuad(&renderer->screenQuad, renderPass, 1, &renderer->gbuffer->depthAttachment, renderer->defaultSampler, cmdBuffer);
		}

		// directional lights
		{
			SDL_BindGPUGraphicsPipeline(renderPass, renderer->directionalLightPipeline->pipeline);

			struct UniformData
			{
				vec4 lightDirection;
				vec4 lightColor;
			};
			UniformData uniforms = {};
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

			SDL_PushGPUVertexUniformData(cmdBuffer, 0, &pv, sizeof(pv));

			struct UniformData
			{
				mat4 projectionViewInv;
				vec4 viewTexel;
			};
			UniformData uniforms = {};
			uniforms.projectionViewInv = pv.inverted();
			uniforms.viewTexel = vec4(1.0f / renderer->hdrTarget->width, 1.0f / renderer->hdrTarget->height, 0, 0);
			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTextureSamplerBinding textureBindings[MAX_COLOR_ATTACHMENTS + 1];
			for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			{
				textureBindings[i].texture = renderer->gbuffer->colorAttachments[i];
				textureBindings[i].sampler = renderer->defaultSampler;
			}
			textureBindings[renderer->gbuffer->numColorAttachments].texture = renderer->gbuffer->depthAttachment;
			textureBindings[renderer->gbuffer->numColorAttachments].sampler = renderer->defaultSampler;

			SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, renderer->gbuffer->numColorAttachments + 1);

			SDL_DrawGPUIndexedPrimitives(renderPass, renderer->cubeIndexBuffer->numIndices, renderer->pointLights.size, 0, 0, 0);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	// tonemapping
	{
		SDL_GPUColorTargetInfo colorTarget = {};
		colorTarget.load_op = SDL_GPU_LOADOP_DONT_CARE;
		colorTarget.store_op = SDL_GPU_STOREOP_STORE;
		colorTarget.texture = swapchain;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTarget, 1, nullptr);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->tonemappingPipeline->pipeline);

		RenderScreenQuad(&renderer->screenQuad, renderPass, 1, &renderer->hdrTarget->colorAttachments[0], renderer->defaultSampler, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	renderer->meshes.clear();
	renderer->animatedMeshes.clear();
	renderer->pointLights.clear();
}
