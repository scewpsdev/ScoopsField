#include "GPUTiming.h"


#ifdef GPU_TIMING

#define GPU_TIMER_RESET 5000


void InitGPUTimer(GpuTimerContext* ctx, SDL_GPUDevice* device)
{
	VulkanRenderer* vk = (VulkanRenderer*)device->driverData;

	SDL_memset(ctx, 0, sizeof(*ctx));
	ctx->device = vk->logicalDevice;
	ctx->physicalDevice = vk->physicalDevice;

	VkPhysicalDeviceProperties props;
	vk->vkGetPhysicalDeviceProperties(vk->physicalDevice, &props);
	ctx->timestampPeriod = props.limits.timestampPeriod;

	for (int i = 0; i < GPU_TIMER_FRAMES; i++)
	{
		VkQueryPoolCreateInfo qp = {};
		qp.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		qp.queryType = VK_QUERY_TYPE_TIMESTAMP;
		qp.queryCount = GPU_TIMER_MAX_QUERIES;

		vk->vkCreateQueryPool(vk->logicalDevice, &qp, NULL, &ctx->frames[i].queryPool);
	}
}

static GpuTimerFrame* GetFrame(GpuTimerContext* ctx)
{
	return &ctx->frames[ctx->frameIndex];
}

void BeginGPUTimerFrame(GpuTimerContext* ctx, SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmdBuffer)
{
	ctx->frameIndex = (ctx->frameIndex + 1) % GPU_TIMER_FRAMES;

	GpuTimerFrame* f = GetFrame(ctx);
	f->queryCount = 0;
	f->nodeCount = 0;
	f->stackSize = 0;

	VulkanRenderer* vk = (VulkanRenderer*)device->driverData;
	vk->vkCmdResetQueryPool(((VulkanCommandBuffer*)cmdBuffer)->commandBuffer, f->queryPool, 0, GPU_TIMER_MAX_QUERIES);
}

void BeginGPUTimer(SDL_GPUCommandBuffer* cmdBuffer, const char* label)
{
	GpuTimerContext* ctx = &app->gpuTiming;
	GpuTimerFrame* f = GetFrame(ctx);

	uint32_t nodeIndex = f->nodeCount++;
	GpuTimerNode* node = &f->nodes[nodeIndex];

	node->label = label;
	node->firstChild = UINT32_MAX;
	node->lastChild = UINT32_MAX;
	node->nextSibling = UINT32_MAX;

	// hierarchy
	if (f->stackSize > 0)
	{
		uint32_t parent = f->stack[f->stackSize - 1];
		node->parent = parent;

		GpuTimerNode* p = &f->nodes[parent];
		if (p->lastChild != UINT32_MAX)
			f->nodes[p->lastChild].nextSibling = nodeIndex;
		if (p->firstChild == UINT32_MAX)
			p->firstChild = nodeIndex;
		p->lastChild = nodeIndex;
		//node->nextSibling = p->firstChild;
		//p->firstChild = nodeIndex;
	}
	else
	{
		node->parent = UINT32_MAX; // root
	}

	// timestamp
	uint32_t q = f->queryCount++;
	node->startQuery = q;

	VulkanRenderer* vk = (VulkanRenderer*)device->driverData;
	vk->vkCmdWriteTimestamp(((VulkanCommandBuffer*)cmdBuffer)->commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, f->queryPool, q);

	// push stack
	f->stack[f->stackSize++] = nodeIndex;
}

void EndGPUTimer(SDL_GPUCommandBuffer* cmdBuffer)
{
	GpuTimerContext* ctx = &app->gpuTiming;
	GpuTimerFrame* f = GetFrame(ctx);

	uint32_t nodeIndex = f->stack[--f->stackSize];
	GpuTimerNode* node = &f->nodes[nodeIndex];

	uint32_t q = f->queryCount++;
	node->endQuery = q;

	VulkanRenderer* vk = (VulkanRenderer*)device->driverData;
	vk->vkCmdWriteTimestamp(((VulkanCommandBuffer*)cmdBuffer)->commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, f->queryPool, q);
}

static void ResetGPUTimers(GpuTimerContext* ctx)
{
	ClearHashMap(&ctx->cumulativeGpuTimes);
	ctx->numCumulativeFrames = 0;
}

void ResolveGPUTimers(GpuTimerContext* ctx, SDL_GPUDevice* device)
{
	if (SDL_GetTicks() - ctx->lastTimerReset > GPU_TIMER_RESET)
	{
		ResetGPUTimers(ctx);
		ctx->lastTimerReset = SDL_GetTicks();
	}

	uint32_t resolveFrame = (ctx->frameIndex + GPU_TIMER_FRAMES - 1) % GPU_TIMER_FRAMES;
	GpuTimerFrame* f = &ctx->frames[resolveFrame];

	if (f->queryCount == 0) return;

	uint64_t timestamps[GPU_TIMER_MAX_QUERIES];

	VulkanRenderer* vk = (VulkanRenderer*)device->driverData;
	vk->vkGetQueryPoolResults(
		ctx->device,
		f->queryPool,
		0,
		f->queryCount,
		sizeof(uint64_t) * f->queryCount,
		timestamps,
		sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
	);

	for (uint32_t i = 0; i < f->nodeCount; i++)
	{
		GpuTimerNode* n = &f->nodes[i];

		uint64_t start = timestamps[n->startQuery];
		uint64_t end = timestamps[n->endQuery];

		n->timeMs = (end - start) * ctx->timestampPeriod / 1e6f;

		float* cumulativeTime = HashMapGet(&ctx->cumulativeGpuTimes, n->label);
		if (!cumulativeTime)
			cumulativeTime = HashMapAdd(&ctx->cumulativeGpuTimes, n->label, n->timeMs * ctx->numCumulativeFrames);

		*cumulativeTime += n->timeMs;
	}

	ctx->numCumulativeFrames++;
}

static void PrintNode(GpuTimerContext* ctx, GpuTimerFrame* f, int x, int& y, uint32_t index, int depth)
{
	GpuTimerNode* n = &f->nodes[index];

	float* cumulativeTime = HashMapGet(&ctx->cumulativeGpuTimes, n->label);
	SDL_assert(cumulativeTime);
	float avgTime = cumulativeTime ? *cumulativeTime / ctx->numCumulativeFrames : n->timeMs;

	DebugText(x, y, 0x0, 0xFF000000, "%.*s", depth * 2, "                ");
	DebugText(x + depth * 2, y, 0xFFFFFFFF, 0xFF000000, "%s: %.3f ms", n->label, avgTime);
	y++;

	uint32_t child = n->firstChild;
	while (child != UINT32_MAX)
	{
		PrintNode(ctx, f, x, y, child, depth + 1);
		child = f->nodes[child].nextSibling;
	}
}

void PrintGPUTimers(GpuTimerContext* ctx, int x, int y)
{
	uint32_t readFrame = (ctx->frameIndex + GPU_TIMER_FRAMES - 2) % GPU_TIMER_FRAMES;
	GpuTimerFrame* f = &ctx->frames[readFrame];

	for (uint32_t i = 0; i < f->nodeCount; i++)
	{
		if (f->nodes[i].parent == UINT32_MAX)
			PrintNode(ctx, f, x, y, i, 0);
	}
}

#else

void InitGPUTimer(GpuTimerContext* ctx, SDL_GPUDevice* device)
{
}

void BeginGPUTimerFrame(GpuTimerContext* ctx, SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmdBuffer)
{
}

void BeginGPUTimer(SDL_GPUCommandBuffer* cmdBuffer, const char* label)
{
}

void EndGPUTimer(SDL_GPUCommandBuffer* cmdBuffer)
{
}

void ResolveGPUTimers(GpuTimerContext* ctx, SDL_GPUDevice* device)
{
}

void PrintGPUTimers(GpuTimerContext* ctx, int x, int y)
{
}

#endif
