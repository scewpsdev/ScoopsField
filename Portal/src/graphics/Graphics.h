#pragma once

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/IndirectBuffer.h"
#include "graphics/StorageBuffer.h"
#include "graphics/TransferBuffer.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/RenderTarget.h"
#include "graphics/GraphicsPipeline.h"

#include "utils/Pool.h"


struct GraphicsState
{
#define MAX_VERTEX_BUFFERS 4096
	Pool<VertexBuffer, MAX_VERTEX_BUFFERS> vertexBuffers;

#define MAX_INDEX_BUFFERS 1024
	Pool<IndexBuffer, MAX_INDEX_BUFFERS> indexBuffers;

#define MAX_INDIRECT_BUFFERS 16
	Pool<IndirectBuffer, MAX_INDIRECT_BUFFERS> indirectBuffers;

#define MAX_STORAGE_BUFFERS 64
	Pool<StorageBuffer, MAX_STORAGE_BUFFERS> storageBuffers;

#define MAX_TRANSFER_BUFFERS 256
	Pool<TransferBuffer, MAX_TRANSFER_BUFFERS> transferBuffers;

#define MAX_SHADERS 256
	Pool<Shader, MAX_SHADERS> shaders;

#define MAX_TEXTURES 1024
	Pool<Texture, MAX_TEXTURES> textures;

#define MAX_RENDER_TARGETS 256
	Pool<RenderTarget, MAX_RENDER_TARGETS> renderTargets;

#define MAX_GRAPHICS_PIPELINES 64
	Pool<GraphicsPipeline, MAX_GRAPHICS_PIPELINES> graphicsPipelines;
};


void InitGraphics(GraphicsState* graphics);
