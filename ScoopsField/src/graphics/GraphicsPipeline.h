#pragma once

#include <SDL3/SDL.h>

#include "VertexBuffer.h"
#include "Shader.h"


struct GraphicsPipelineInfo
{
	Shader* shader;
	SDL_GPUPrimitiveType primitiveType;
	SDL_GPUCullMode cullMode;

#define MAX_PIPELINE_COLOR_TARGETS 8
	int numColorTargets;
	SDL_GPUColorTargetDescription colorTargets[MAX_PIPELINE_COLOR_TARGETS];

	bool hasDepthTarget;
	SDL_GPUTextureFormat depthFormat;

#define MAX_PIPELINE_VERTEX_ATTRIBUTES 8
	int numAttributes;
	SDL_GPUVertexAttribute attributes[MAX_PIPELINE_VERTEX_ATTRIBUTES];

#define MAX_PIPELINE_VERTEX_BUFFERS 8
	int numVertexBuffers;
	SDL_GPUVertexBufferDescription bufferDescriptions[MAX_PIPELINE_VERTEX_BUFFERS];
};

struct GraphicsPipeline
{
	SDL_GPUGraphicsPipeline* pipeline;
	GraphicsPipelineInfo pipelineInfo;
};


GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo* pipelineInfo);
void DestroyGraphicsPipeline(GraphicsPipeline* pipeline);

void ReloadGraphicsPipeline(GraphicsPipeline* pipeline);
GraphicsPipelineInfo CreateGraphicsPipelineInfo(Shader* shader, int numVertexBuffers, const VertexBufferLayout* vertexLayouts);
