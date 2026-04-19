#include "Resource.h"

#include "Application.h"


extern AppState* app;
extern ResourceState* resource;

extern SDL_GPUCommandBuffer* cmdBuffer;


void InitResourceState(ResourceState* resource)
{
	InitHashMap(&resource->modelNameMap);
	InitAnimationCache(&resource->animationCache);
}

void AddFileWatcher(const char* path)
{
	if (resource->numFileWatchers < MAX_FILE_WATCHERS)
	{
		FileWatcher* watcher = &resource->fileWatchers[resource->numFileWatchers];

		SDL_PathInfo pathInfo = {};
		if (SDL_GetPathInfo(path, &pathInfo))
		{
			SDL_strlcpy(watcher->path, path, sizeof(watcher->path));
			watcher->lastWriteTime = pathInfo.modify_time;

			resource->numFileWatchers++;
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
		}
	}
}

static FileWatcher* GetFileWatcherFromPath(const char* path)
{
	for (int i = 0; i < resource->numFileWatchers; i++)
	{
		if (SDL_strcmp(path, resource->fileWatchers[i].path) == 0)
		{
			return &resource->fileWatchers[i];
		}
	}
	return nullptr;
}

bool FileHasChanged(const char* path)
{
	if (FileWatcher* watcher = GetFileWatcherFromPath(path))
	{
		SDL_PathInfo pathInfo = {};
		if (SDL_GetPathInfo(watcher->path, &pathInfo))
		{
			if (pathInfo.modify_time > watcher->lastWriteTime)
			{
				watcher->lastWriteTime = pathInfo.modify_time;
				return true;
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
		}
	}
	else
	{
		SDL_assert(false);
	}
	return false;
}

StringView GetDirectory(const char* path)
{
	const char* slash = max(SDL_strrchr(path, '/'), SDL_strrchr(path, '\\'));
	int length = slash ? (int)(slash - path) : 0;
	return StringView{ slash ? path : nullptr, length };
}

void GetAbsolutePath(char* str, int maxLen, const char* relativePath, const char* relativeTo)
{
	StringView directory = GetDirectory(relativeTo);
	SDL_snprintf(str, maxLen, "%.*s/%s", directory.buffer ? directory.length : 1, directory.buffer ? directory.buffer : ".", relativePath);
}

void GetRelativePath(char* str, int maxLen, const char* absolutePath, const char* relativeTo)
{
	StringView directory = GetDirectory(relativeTo);
	const char* relativePath = directory.buffer ? absolutePath + directory.length + 1 : absolutePath;
	SDL_snprintf(str, maxLen, "%s", relativePath);
}

Model* GetModel(const char* path)
{
	uint32_t pathHash = hash(path);
	if (int* modelID = HashMapGet(&resource->modelNameMap, pathHash))
		return &resource->models[*modelID];
	else
	{
		char fullPath[256];
		SDL_snprintf(fullPath, 256, "res/%s.bin", path);

		if (LoadModel(&resource->models[resource->numModels], fullPath, false, cmdBuffer))
		{
			int modelID = resource->numModels++;
			Model* model = &resource->models[modelID];
			HashMapAdd(&resource->modelNameMap, pathHash, modelID);
			return model;
		}

		return nullptr;
	}
}
