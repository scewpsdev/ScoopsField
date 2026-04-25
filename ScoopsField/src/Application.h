#pragma once

#define GPU_TIMING

#include <SDL3/SDL.h>

#include <stdlib.h>

#include "GameMemory.h"

#include "Resource.h"

#include "game/Game.h"

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/IndirectBuffer.h"
#include "graphics/StorageBuffer.h"
#include "graphics/TransferBuffer.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/RenderTarget.h"
#include "graphics/GraphicsPipeline.h"

#include "graphics/GPUTiming.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "math/Math.h"
#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"

#include "utils/BumpAllocator.h"
#include "utils/Queue.h"
#include "utils/Pool.h"
#include "utils/HashMap.h"


#define PROJECT_PATH "D:\\Dev\\ScoopsField\\ScoopsField"


struct PlatformCallbacks
{
	void (*compileResources)();
};

struct GraphicsState
{
#define MAX_VERTEX_BUFFERS 4096
	VertexBuffer vertexBuffers[MAX_VERTEX_BUFFERS];
	int numVertexBuffers;

#define MAX_INDEX_BUFFERS 1024
	IndexBuffer indexBuffers[MAX_INDEX_BUFFERS];
	int numIndexBuffers;

#define MAX_INDIRECT_BUFFERS 16
	IndirectBuffer indirectBuffers[MAX_INDIRECT_BUFFERS];
	int numIndirectBuffers;

#define MAX_STORAGE_BUFFERS 64
	StorageBuffer storageBuffers[MAX_STORAGE_BUFFERS];
	int numStorageBuffers;

#define MAX_TRANSFER_BUFFERS 64
	TransferBuffer transferBuffers[MAX_TRANSFER_BUFFERS];
	int numTransferBuffers;

#define MAX_SHADERS 256
	Shader shaders[MAX_SHADERS];
	int numShaders;

#define MAX_TEXTURES 1024
	Texture textures[MAX_TEXTURES];
	int numTextures;

#define MAX_RENDER_TARGETS 256
	RenderTarget renderTargets[MAX_RENDER_TARGETS];
	int numRenderTargets;

#define MAX_GRAPHICS_PIPELINES 64
	GraphicsPipeline graphicsPipelines[MAX_GRAPHICS_PIPELINES];
	int numGraphicsPipelines;
};

struct AppState
{
	PlatformCallbacks platformCallbacks;

	SDL_Window* window;
	SDL_GPUDevice* device;

	GpuTimerContext gpuTiming;

	uint64_t now;
	uint64_t lastFrame;
	uint64_t lastSecond;
	uint64_t frameTime;
	uint64_t frameTimeVariance;
	int frameIdx;
	int lastSecondFrame;

	int width, height;
	bool debugStats;
	int fps;
	float avgMs;
	float avgMsVariance;
	int platformAllocationsPerFrame;
	int physicsAllocationsPerFrame;

	float deltaTime;
	float gameTime;

	int numKeys;
	const bool* keys;
	bool* lastKeys;

	vec2 mousePosition;
	vec2 lastMousePosition;
	vec2 mouseDelta;
	SDL_MouseButtonFlags mouseButtons;
	SDL_MouseButtonFlags lastMouseButtons;

	SoLoud::Soloud* soloud;

	GraphicsState graphics;
	AudioState audio;
	PhysicsState physics;
	ResourceState resourceState;
	GameState game;

	DebugTextRenderer debugTextRenderer;
};
