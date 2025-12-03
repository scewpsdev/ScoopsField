#include <Windows.h>
#undef min
#undef max

#include <SDL3/SDL.h>

#include "Application.h"

#include "utils/BumpAllocator.h"
#include "utils/HashMap.h"


#define Kilobytes(x) ((x) * 1024LL)
#define Megabytes(x) (Kilobytes(x) * 1024LL)
#define Gigabytes(x) (Megabytes(x) * 1024LL)
#define Terabytes(x) (Gigabytes(x) * 1024LL)


static GameMemory memory;
static HashMap<void*, uint64_t, 1000> platformAllocations;

static SDL_malloc_func defaultMalloc;
static SDL_calloc_func defaultCalloc;
static SDL_realloc_func defaultRealloc;
static SDL_free_func defaultFree;


extern "C" __declspec(dllexport) SDL_AppResult AppInit(GameMemory* memory, AppState* appState, int argc, char** argv);
extern "C" __declspec(dllexport) void AppDestroy(GameMemory* memory, AppState* appState, SDL_AppResult result);
extern "C" __declspec(dllexport) SDL_AppResult AppOnEvent(GameMemory* memory, AppState* appState, SDL_Event* event);
extern "C" __declspec(dllexport) void AppIterate(GameMemory* memory, AppState* appState);


SDL_Time GetWriteTime(const char* path)
{
	SDL_PathInfo pathInfo = {};
	if (SDL_GetPathInfo(path, &pathInfo))
		return pathInfo.modify_time;
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
		return 0;
	}
}

static void CompileResources()
{
	int result = system("D:\\Dev\\Rainfall\\RainfallResourceCompiler\\bin\\x64\\Release\\RainfallResourceCompiler.exe " PROJECT_PATH "\\res res png ogg vsh fsh csh glsl vert frag comp ttf rfs gltf glb");
	SDL_assert(result == 0);
}

static void InitPlatformCallbacks(PlatformCallbacks* callbacks)
{
	callbacks->compileResources = CompileResources;
}

void* SDLmalloc(size_t size)
{
	memory.platformMemoryUsage += size;
	memory.platformAllocationCount++;
	void* mem = defaultMalloc(size);
	HashMapAdd(&platformAllocations, mem, size);
	return mem;
}

void* SDLcalloc(size_t nmemb, size_t size)
{
	memory.platformMemoryUsage += nmemb * size;
	memory.platformAllocationCount++;
	void* mem = defaultCalloc(nmemb, size);
	HashMapAdd(&platformAllocations, mem, nmemb * size);
	return mem;
}

void* SDLrealloc(void* mem, size_t size)
{
	if (!mem)
		return SDL_malloc(size);
	else if (!size)
	{
		SDL_free(mem);
		return nullptr;
	}

	void* newMem = defaultRealloc(mem, size);
	if (newMem == mem)
	{
		uint64_t* memsize = HashMapGet(&platformAllocations, mem);
		memory.platformMemoryUsage += size - *memsize;
		*memsize = size;
	}
	else
	{
		uint64_t* memsize = HashMapRemove(&platformAllocations, mem);
		memory.platformMemoryUsage += size - *memsize;
		HashMapAdd(&platformAllocations, newMem, size);
	}
	return newMem;
}

void SDLfree(void* mem)
{
	defaultFree(mem);
	uint64_t* memsize = HashMapRemove(&platformAllocations, mem);
	memory.platformMemoryUsage -= *memsize;
	memory.platformAllocationCount--;
}

int main(int argc, char** argv)
{
	InitHashMap(&platformAllocations);

	memory = {};
	memory.constantMemorySize = Gigabytes(2);
	memory.transientMemorySize = Megabytes(256);

	void* baseAddress = (void*)Terabytes(2);
	uint64_t totalSize = memory.constantMemorySize + memory.transientMemorySize;
	memory.constantMemory = (uint8_t*)VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memory.transientMemory = memory.constantMemory + memory.constantMemorySize;

	InitBumpAllocator(&memory.constantAllocator, memory.constantMemory, memory.constantMemorySize);
	InitBumpAllocator(&memory.transientAllocator, memory.transientMemory, memory.transientMemorySize);

	AppState* appState = (AppState*)(BumpAllocatorMalloc(&memory.constantAllocator, sizeof(AppState)));
	InitPlatformCallbacks(&appState->platformCallbacks);

#if _DEBUG
	SDL_GetOriginalMemoryFunctions(&defaultMalloc, &defaultCalloc, &defaultRealloc, &defaultFree);
	SDL_SetMemoryFunctions(SDLmalloc, SDLcalloc, SDLrealloc, SDLfree);
#endif

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_Log("SDL %s", SDL_GetRevision());

	const char* title = "abc";
	int width = 1280;
	int height = 720;
	SDL_Window* window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	if (!window)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s", SDL_GetError());
		return 1;
	}
	SDL_Log("Display %dx%d", width, height);

	bool gpuDebug = false;
#if _DEBUG
	gpuDebug = true;
#endif

	SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, gpuDebug, nullptr);
	if (!device)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_GPU, "Failed to create graphics device: %s", SDL_GetError());
		return 1;
	}
	if (!SDL_ClaimWindowForGPUDevice(device, window))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_GPU, "Failed to create swapchain: %s", SDL_GetError());
		return 1;
	}

	SDL_Log("Video driver %s-%s", SDL_GetCurrentVideoDriver(), SDL_GetGPUDeviceDriver(device));

	SDL_GPUSwapchainComposition swapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
	SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
	if (!SDL_SetGPUSwapchainParameters(device, window, swapchainComposition, presentMode))
	{
		SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to set swapchain parameters: %s", SDL_GetError());
	}

	appState->window = window;
	appState->device = device;

	SDL_Log("Loading game code\n");

	SDL_AppResult result = AppInit(&memory, appState, argc, argv);

	if (!SDL_ShowWindow(window))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
		return 1;
	}

	SDL_Log("Initialization complete");


	if (result == SDL_APP_CONTINUE)
	{
		bool running = true;
		while (running)
		{
			SDL_Event event = {};
			while (SDL_PollEvent(&event))
			{
				SDL_AppResult result = AppOnEvent(&memory, appState, &event);
				if (result != SDL_APP_CONTINUE)
					running = false;
			}

			AppIterate(&memory, appState);

			ResetBumpAllocator(&memory.transientAllocator);
		}
	}

	AppDestroy(&memory, appState, result);

	VirtualFree(memory.constantMemory, 0, MEM_RELEASE);

	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
