#include "GraphicsPipeline.h"

#include "Application.h"
#include "VertexBuffer.h"
#include "Shader.h"


extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo* pipelineInfo)
{
	GraphicsPipeline* pipeline = &graphics->graphicsPipelines[graphics->numGraphicsPipelines++];
	pipeline->pipelineInfo = *pipelineInfo;

	ReloadGraphicsPipeline(pipeline);

	return pipeline;
}

void DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
	SDL_ReleaseGPUGraphicsPipeline(device, pipeline->pipeline);
	pipeline->pipeline = nullptr;
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

	createInfo.target_info.num_color_targets = pipeline->pipelineInfo.numColorTargets;
	createInfo.target_info.color_target_descriptions = pipeline->pipelineInfo.colorTargets;

	createInfo.target_info.has_depth_stencil_target = pipelineInfo->hasDepthTarget;
	createInfo.target_info.depth_stencil_format = pipelineInfo->depthFormat;

	createInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
	createInfo.depth_stencil_state.enable_depth_test = pipelineInfo->hasDepthTarget;
	createInfo.depth_stencil_state.enable_depth_write = pipelineInfo->hasDepthTarget;

	createInfo.vertex_input_state.num_vertex_attributes = pipeline->pipelineInfo.numAttributes;
	createInfo.vertex_input_state.vertex_attributes = pipeline->pipelineInfo.attributes;

	createInfo.vertex_input_state.num_vertex_buffers = pipeline->pipelineInfo.numVertexBuffers;
	createInfo.vertex_input_state.vertex_buffer_descriptions = pipeline->pipelineInfo.bufferDescriptions;

	pipeline->pipeline = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
}

GraphicsPipelineInfo CreateGraphicsPipelineInfo(Shader* shader, int numVertexBuffers, const VertexBufferLayout* vertexLayouts)
{
	GraphicsPipelineInfo pipelineInfo = {};

	pipelineInfo.shader = shader;
	pipelineInfo.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineInfo.cullMode = SDL_GPU_CULLMODE_BACK;

	pipelineInfo.numColorTargets = 1;
	pipelineInfo.colorTargets[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);
	pipelineInfo.colorTargets[0].blend_state.enable_blend = true;
	pipelineInfo.colorTargets[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	pipelineInfo.colorTargets[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	pipelineInfo.colorTargets[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	pipelineInfo.colorTargets[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineInfo.colorTargets[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	pipelineInfo.colorTargets[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

	pipelineInfo.hasDepthTarget = true;
	pipelineInfo.depthFormat = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;

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
