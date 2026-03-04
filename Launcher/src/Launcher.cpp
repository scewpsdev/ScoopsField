#include <Windows.h>
#undef min
#undef max

#include <SDL3/SDL.h>

#include "GameMemory.h"

#include "utils/BumpAllocator.h"
#include "utils/HashMap.h"


#define GAME_NAME "ScoopsField"

#define GAME_CODE_DLL GAME_NAME ".dll"

#define Kilobytes(x) ((x) * 1024LL)
#define Megabytes(x) (Kilobytes(x) * 1024LL)
#define Gigabytes(x) (Megabytes(x) * 1024LL)
#define Terabytes(x) (Gigabytes(x) * 1024LL)


typedef SDL_AppResult(*AppInit_t)(GameMemory* memory, int argc, char** argv);
typedef void(*AppDestroy_t)(SDL_AppResult result);
typedef SDL_AppResult(*AppIterate_t)();
typedef void(*AppReload_t)(GameMemory* memory);


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

static SDL_SharedObject* LoadGameCode(AppInit_t* init, AppDestroy_t* destroy, AppIterate_t* iterate, AppReload_t* reload)
{
	char tmpPath[256];
	SDL_snprintf(tmpPath, 256, GAME_NAME "-%llu.dll", SDL_GetPerformanceCounter());
	bool copySuccessful = false;
	for (int i = 0; i < 50; i++)
	{
		if (SDL_CopyFile(GAME_CODE_DLL, tmpPath))
		{
			copySuccessful = true;
			break;
		}
		Sleep(10);
		SDL_Log("retrying");
	}
	if (!copySuccessful)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to copy game code module: %s", SDL_GetError());
		return nullptr;
	}

	SDL_SharedObject* gameCode = SDL_LoadObject(tmpPath);
	if (!gameCode)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load game code module: %s", SDL_GetError());
		return nullptr;
	}

	*init = (AppInit_t)SDL_LoadFunction(gameCode, "AppInit");
	*destroy = (AppDestroy_t)SDL_LoadFunction(gameCode, "AppDestroy");
	*iterate = (AppIterate_t)SDL_LoadFunction(gameCode, "AppIterate");
	*reload = (AppReload_t)SDL_LoadFunction(gameCode, "AppReload");

	if (!*init)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load AppInit: %s", SDL_GetError());
		return nullptr;
	}
	if (!*destroy)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load AppDestroy: %s", SDL_GetError());
		return nullptr;
	}
	if (!*iterate)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load AppIterate: %s", SDL_GetError());
		return nullptr;
	}
	if (!*reload)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load AppReload: %s", SDL_GetError());
		return nullptr;
	}

	return gameCode;
}

int main(int argc, char** argv)
{
	GameMemory memory = {};

	memory.constantMemorySize = Megabytes(64);
	memory.transientMemorySize = Megabytes(16);

	void* baseAddress = (void*)Terabytes(2);
	uint64_t totalSize = memory.constantMemorySize + memory.transientMemorySize;
	memory.constantMemory = (uint8_t*)VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memory.transientMemory = memory.constantMemory + memory.constantMemorySize;

	AppInit_t appInit = nullptr;
	AppDestroy_t appDestroy = nullptr;
	AppIterate_t appIterate = nullptr;
	AppReload_t appReload = nullptr;

	SDL_SharedObject* gameCode = LoadGameCode(&appInit, &appDestroy, &appIterate, &appReload);
	if (!gameCode)
		return 1;

	SDL_Time lastGameCodeWrite = GetWriteTime(GAME_CODE_DLL);

	SDL_AppResult result = appInit(&memory, argc, argv);
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
		SDL_Time gameCodeWriteTime = GetWriteTime(GAME_CODE_DLL);
		if (gameCodeWriteTime > lastGameCodeWrite)
		{
			SDL_Log("Reloading game code\n");

			//SDL_UnloadObject(gameCode);
			if (SDL_SharedObject* newGameCode = LoadGameCode(&appInit, &appDestroy, &appIterate, &appReload))
				gameCode = newGameCode;

			appReload(&memory);

			lastGameCodeWrite = gameCodeWriteTime;
		}

		SDL_AppResult result = appIterate();
		if (result != SDL_APP_CONTINUE)
			running = false;
	}


	SDL_Log("Shutting down");

	appDestroy(result);

	SDL_UnloadObject(gameCode);

	VirtualFree(memory.constantMemory, 0, MEM_RELEASE);

	system("del " GAME_NAME "-*.pdb");
	system("del " GAME_NAME "-*.dll");

	return 0;
}
