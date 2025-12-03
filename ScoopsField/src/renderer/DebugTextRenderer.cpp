#include "DebugTextRenderer.h"

#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/Math.h"

#include <SDL3/SDL.h>



extern SDL_Window* window;
extern SDL_GPUDevice* device;
extern SDL_GPUTexture* swapchain;


void InitDebugTextRenderer(DebugTextRenderer* renderer, int maxGlyphs, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->maxGlyphs = maxGlyphs;

	renderer->shader = LoadGraphicsShader("res/shaders/glyph.vert.bin", "res/shaders/glyph.frag.bin");
	renderer->texture = LoadTexture("res/textures/debugfont.png.bin", cmdBuffer);

	{
		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.vertex_shader = renderer->shader->vertex;
		pipelineInfo.fragment_shader = renderer->shader->fragment;
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

		renderer->pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
	}

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->sampler = SDL_CreateGPUSampler(device, &samplerInfo);

	SDL_GPUBufferCreateInfo glyphDataBufferInfo = {};
	glyphDataBufferInfo.size = maxGlyphs * sizeof(GlyphData);
	glyphDataBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;

	renderer->glyphDataBuffer = SDL_CreateGPUBuffer(device, &glyphDataBufferInfo);

	SDL_GPUTransferBufferCreateInfo transferBufferInfo = {};
	transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBufferInfo.size = maxGlyphs * sizeof(GlyphData);

	renderer->transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);
}

void DestroyDebugTextRenderer(DebugTextRenderer* renderer)
{
	SDL_ReleaseGPUBuffer(device, renderer->glyphDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, renderer->transferBuffer);

	SDL_ReleaseGPUGraphicsPipeline(device, renderer->pipeline);

	DestroyTexture(renderer->texture);
	DestroyShader(renderer->shader);

	SDL_ReleaseGPUSampler(device, renderer->sampler);
	SDL_ReleaseGPUGraphicsPipeline(device, renderer->pipeline);
}

void DebugTextRendererBegin(DebugTextRenderer* renderer)
{
	renderer->glyphDataPtr = (GlyphData*)SDL_MapGPUTransferBuffer(device, renderer->transferBuffer, true);
	renderer->numGlyphs = 0;
}

void DebugTextRendererSubmit(DebugTextRenderer* renderer, int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor)
{
	SDL_assert(renderer->numGlyphs + len <= renderer->maxGlyphs);

	for (int i = 0; i < len; i++)
	{
		int glyphID = txt[i] - 32;
		int u = glyphID % 16;
		int v = glyphID / 16;

		vec4 rect = vec4(u / 16.0f, v / 8.0f, 1 / 16.0f, 1 / 8.0f);

		GlyphData* glyphData = &renderer->glyphDataPtr[renderer->numGlyphs++];

		glyphData->position = vec2((x + i) * 8.0f, y * 16.0f);
		glyphData->rect = rect;
		glyphData->color = ARGBToVector(color);
		glyphData->bgcolor = ARGBToVector(bgcolor);
	}
}

void DebugTextRendererEnd(DebugTextRenderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (renderer->numGlyphs == 0)
		return;


	SDL_UnmapGPUTransferBuffer(device, renderer->transferBuffer);
	renderer->glyphDataPtr = nullptr;

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	SDL_GPUTransferBufferLocation location = {};
	location.transfer_buffer = renderer->transferBuffer;
	location.offset = 0;

	SDL_GPUBufferRegion region = {};
	region.buffer = renderer->glyphDataBuffer;
	region.size = renderer->numGlyphs * sizeof(GlyphData);
	region.offset = 0;

	SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
	SDL_EndGPUCopyPass(copyPass);


	SDL_GPUColorTargetInfo colorTarget = {};
	colorTarget.load_op = SDL_GPU_LOADOP_LOAD;
	colorTarget.store_op = SDL_GPU_STOREOP_STORE;
	colorTarget.texture = swapchain;

	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTarget, 1, nullptr);

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->pipeline);

	SDL_BindGPUVertexStorageBuffers(renderPass, 0, &renderer->glyphDataBuffer, 1);

	mat4 projection = mat4::Orthographic(0, (float)width, (float)height, 0, 0, 1);
	SDL_PushGPUVertexUniformData(cmdBuffer, 0, &projection, sizeof(projection));

	SDL_GPUTextureSamplerBinding binding = {};
	binding.texture = renderer->texture->handle;
	binding.sampler = renderer->sampler;

	SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

	SDL_DrawGPUPrimitives(renderPass, renderer->numGlyphs * 6, 1, 0, 0);

	SDL_EndGPURenderPass(renderPass);
}
