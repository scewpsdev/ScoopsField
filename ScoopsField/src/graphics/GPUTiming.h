#pragma once

#ifdef GPU_TIMING

#include "GPUVulkan.h"

#include "utils/HashMap.h"
#include "utils/Defer.h"


#define GPU_SCOPE(name) \
	BeginGPUTimer(cmdBuffer, name); \
	defer( EndGPUTimer(cmdBuffer); );


#define GPU_TIMER_MAX_QUERIES 128
#define GPU_TIMER_MAX_LABELS  64
#define GPU_TIMER_FRAMES      3


struct GpuTimerNode
{
	const char* label;

	uint32_t startQuery;
	uint32_t endQuery;

	uint32_t parent;
	uint32_t firstChild;
	uint32_t lastChild;
	uint32_t nextSibling;

	float timeMs;
};

struct GpuTimerFrame
{
	VkQueryPool queryPool;
	uint32_t queryCount;

	GpuTimerNode nodes[GPU_TIMER_MAX_LABELS];
	uint32_t nodeCount;

	uint32_t stack[GPU_TIMER_MAX_LABELS];
	uint32_t stackSize;
};

struct GpuTimerContext
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	float timestampPeriod;

	GpuTimerFrame frames[GPU_TIMER_FRAMES];
	uint32_t frameIndex;

	HashMap<const char*, float, GPU_TIMER_MAX_LABELS> cumulativeGpuTimes;
	int numCumulativeFrames;
	int64_t lastTimerReset;
};

#else

struct GpuTimerContext
{
};

#define GPU_SCOPE(name)

#endif


void BeginGPUTimer(SDL_GPUCommandBuffer* cmdBuffer, const char* label);
void EndGPUTimer(SDL_GPUCommandBuffer* cmdBuffer);
