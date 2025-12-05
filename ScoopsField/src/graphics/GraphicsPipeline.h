#pragma once

#include <SDL3/SDL.h>

#include "VertexBuffer.h"
#include "Shader.h"
#include "RenderTarget.h"


struct GraphicsPipelineInfo
{
	Shader* shader;
	SDL_GPUPrimitiveType primitiveType;
	SDL_GPUCullMode cullMode;
	SDL_GPUFillMode fillMode;
	SDL_GPUFrontFace frontFace;

#define MAX_PIPELINE_COLOR_TARGETS 8
	int numColorTargets;
	SDL_GPUColorTargetDescription colorTargets[MAX_PIPELINE_COLOR_TARGETS];

	bool hasDepthTarget;
	SDL_GPUTextureFormat depthFormat;
	bool depthTest = true;
	bool depthWrite = true;
	SDL_GPUCompareOp compareOp = SDL_GPU_COMPAREOP_LESS;

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

GraphicsPipelineInfo CreateGraphicsPipelineInfo(SDL_GPUPrimitiveType primitiveType, SDL_GPUCullMode cullMode, Shader* shader, RenderTarget* renderTarget, int numVertexBuffers, const VertexBufferLayout* const* vertexLayouts);


inline void CreateBlendStateOpaque(SDL_GPUColorTargetBlendState* blendState)
{
	blendState->enable_blend = false;
}

inline void CreateBlendStateAlpha(SDL_GPUColorTargetBlendState* blendState)
{
	blendState->enable_blend = true;

	blendState->src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	blendState->dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blendState->color_blend_op = SDL_GPU_BLENDOP_ADD;

	blendState->src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blendState->alpha_blend_op = SDL_GPU_BLENDOP_ADD;
}

inline void CreateBlendStateAddPremultiplied(SDL_GPUColorTargetBlendState* blendState)
{
	blendState->enable_blend = true;

	blendState->src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->color_blend_op = SDL_GPU_BLENDOP_ADD;

	blendState->src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	blendState->dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blendState->alpha_blend_op = SDL_GPU_BLENDOP_ADD;
}
