#include <Windows.h>
#undef min
#undef max

#include <SDL3/SDL.h>

#include "GameMemory.h"

#include "utils/BumpAllocator.h"
#include "utils/HashMap.h"


#define Kilobytes(x) ((x) * 1024LL)
#define Megabytes(x) (Kilobytes(x) * 1024LL)
#define Gigabytes(x) (Megabytes(x) * 1024LL)
#define Terabytes(x) (Gigabytes(x) * 1024LL)


extern "C" SDL_AppResult AppInit(GameMemory* memory, int argc, char** argv);
extern "C" void AppDestroy(SDL_AppResult result);
extern "C" SDL_AppResult AppIterate();
extern "C" void AppReload(GameMemory* memory);


int main(int argc, char** argv)
{
	GameMemory* memory = (GameMemory*)malloc(sizeof(GameMemory));
	memset(memory, 0, sizeof(GameMemory));

	memory->constantMemorySize = Megabytes(64);
	memory->transientMemorySize = Megabytes(16);

	void* baseAddress = (void*)Terabytes(2);
	uint64_t totalSize = memory->constantMemorySize + memory->transientMemorySize;
	memory->constantMemory = (uint8_t*)VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memory->transientMemory = memory->constantMemory + memory->constantMemorySize;

	SDL_AppResult result = AppInit(memory, argc, argv);
	if (result == SDL_APP_FAILURE)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize application");
		return 1;
	}
	else if (result == SDL_APP_SUCCESS)
	{
		return 0;
	}


	SDL_assert(result == SDL_APP_CONTINUE);

	bool running = true;
	while (running)
	{
		SDL_AppResult result = AppIterate();
		if (result != SDL_APP_CONTINUE)
			running = false;
	}


	AppDestroy(result);

	VirtualFree(memory->constantMemory, 0, MEM_RELEASE);

	free(memory);

	return 0;
}
