#include "GraphicsPipeline.h"

#include "Application.h"

#include "VertexBuffer.h"
#include "Shader.h"
#include "RenderTarget.h"


extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo* pipelineInfo)
{
	GraphicsPipeline* pipeline = PoolAlloc(&graphics->graphicsPipelines);
	pipeline->pipelineInfo = *pipelineInfo;

	ReloadGraphicsPipeline(pipeline);

	return pipeline;
}

void DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
	DestroyShader(pipeline->pipelineInfo.shader);

	SDL_ReleaseGPUGraphicsPipeline(device, pipeline->pipeline);
	pipeline->pipeline = nullptr;
	PoolRelease(&graphics->graphicsPipelines, pipeline);
}

void ReloadGraphicsPipeline(GraphicsPipeline* pipeline)
{
	if (pipeline->pipeline)
		SDL_ReleaseGPUGraphicsPipeline(device, pipeline->pipeline);

	GraphicsPipelineInfo* pipelineInfo = &pipeline->pipelineInfo;

	SDL_GPUGraphicsPipelineCreateInfo createInfo = {};
	createInfo.vertex_shader = pipelineInfo->shader->vertex;
	createInfo.fragment_shader = pipelineInfo->shader->fragment;
	createInfo.primitive_type = pipelineInfo->primitiveType;

	createInfo.rasterizer_state.cull_mode = pipelineInfo->cullMode;
	createInfo.rasterizer_state.fill_mode = pipelineInfo->fillMode;
	createInfo.rasterizer_state.front_face = pipelineInfo->frontFace;
	createInfo.rasterizer_state.enable_depth_clip = !pipelineInfo->depthClamp;

	createInfo.target_info.num_color_targets = pipeline->pipelineInfo.numColorTargets;
	createInfo.target_info.color_target_descriptions = pipeline->pipelineInfo.colorTargets;

	createInfo.target_info.has_depth_stencil_target = pipelineInfo->hasDepthTarget;
	createInfo.target_info.depth_stencil_format = pipelineInfo->depthFormat;

	createInfo.depth_stencil_state.compare_op = pipelineInfo->compareOp;
	createInfo.depth_stencil_state.enable_depth_test = pipelineInfo->hasDepthTarget && pipelineInfo->depthTest;
	createInfo.depth_stencil_state.enable_depth_write = pipelineInfo->hasDepthTarget && pipelineInfo->depthWrite;

	createInfo.vertex_input_state.num_vertex_attributes = pipeline->pipelineInfo.numAttributes;
	createInfo.vertex_input_state.vertex_attributes = pipeline->pipelineInfo.attributes;

	createInfo.vertex_input_state.num_vertex_buffers = pipeline->pipelineInfo.numVertexBuffers;
	createInfo.vertex_input_state.vertex_buffer_descriptions = pipeline->pipelineInfo.bufferDescriptions;

	pipeline->pipeline = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
}

GraphicsPipelineInfo CreateGraphicsPipelineInfo(SDL_GPUPrimitiveType primitiveType, SDL_GPUCullMode cullMode, Shader* shader, int numColorAttachments, SDL_GPUTextureFormat* colorAttachmentFormats, bool hasDepthAttachment, SDL_GPUTextureFormat depthAttachmentFormat, int numVertexBuffers, const VertexBufferLayout* vertexLayouts)
{
	GraphicsPipelineInfo pipelineInfo = {};

	pipelineInfo.shader = shader;
	pipelineInfo.primitiveType = primitiveType;
	pipelineInfo.cullMode = cullMode;

	pipelineInfo.fillMode = SDL_GPU_FILLMODE_FILL;
	pipelineInfo.frontFace = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	pipelineInfo.depthClamp = false;
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_LESS;

	pipelineInfo.numColorTargets = numColorAttachments;
	for (int i = 0; i < numColorAttachments; i++)
	{
		pipelineInfo.colorTargets[i].format = colorAttachmentFormats[i];
	}

	if (hasDepthAttachment)
	{
		pipelineInfo.hasDepthTarget = true;
		pipelineInfo.depthFormat = depthAttachmentFormat;
		pipelineInfo.depthTest = true;
		pipelineInfo.depthWrite = true;
	}

	for (int i = 0; i < numVertexBuffers; i++)
		pipelineInfo.numAttributes += vertexLayouts[i].numAttributes;

	int attributeIdx = 0;
	for (int i = 0; i < numVertexBuffers; i++)
	{
		uint32_t offset = 0;
		for (int j = 0; j < vertexLayouts[i].numAttributes; j++)
		{
			const VertexAttribute* attribute = &vertexLayouts[i].attributes[j];
			pipelineInfo.attributes[attributeIdx].buffer_slot = i;
			pipelineInfo.attributes[attributeIdx].location = attribute->location;
			pipelineInfo.attributes[attributeIdx].format = attribute->format;
			pipelineInfo.attributes[attributeIdx].offset = offset;

			offset += GetVertexFormatSize(attribute->format);
			attributeIdx++;
		}
	}

	pipelineInfo.numVertexBuffers = numVertexBuffers;
	for (int i = 0; i < numVertexBuffers; i++)
	{
		pipelineInfo.bufferDescriptions[i].slot = i;
		pipelineInfo.bufferDescriptions[i].input_rate = vertexLayouts[i].perInstance ? SDL_GPU_VERTEXINPUTRATE_INSTANCE : SDL_GPU_VERTEXINPUTRATE_VERTEX;
		pipelineInfo.bufferDescriptions[i].instance_step_rate = 0;
		pipelineInfo.bufferDescriptions[i].pitch = GetVertexPitch(&vertexLayouts[i]);
	}

	return pipelineInfo;
}

GraphicsPipelineInfo CreateGraphicsPipelineInfo(SDL_GPUPrimitiveType primitiveType, SDL_GPUCullMode cullMode, Shader* shader, RenderTarget* renderTarget, int numVertexBuffers, const VertexBufferLayout* vertexLayouts)
{
	int numColorAttachments = 0;
	SDL_GPUTextureFormat colorAttachmentFormats[MAX_COLOR_ATTACHMENTS];
	bool hasDepthAttachment = false;
	SDL_GPUTextureFormat depthAttachmentFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
	if (renderTarget)
	{
		numColorAttachments = renderTarget->numColorAttachments;
		for (int i = 0; i < renderTarget->numColorAttachments; i++)
		{
			colorAttachmentFormats[i] = renderTarget->colorAttachmentInfos[i].format;
		}
		hasDepthAttachment = renderTarget->hasDepthAttachment;
		depthAttachmentFormat = renderTarget->depthAttachmentInfo.format;
	}
	else
	{
		numColorAttachments = 1;
		colorAttachmentFormats[0] = SDL_GetGPUSwapchainTextureFormat(device, window);
	}

	return CreateGraphicsPipelineInfo(primitiveType, cullMode, shader, numColorAttachments, colorAttachmentFormats, hasDepthAttachment, depthAttachmentFormat, numVertexBuffers, vertexLayouts);
}
