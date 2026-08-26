#include "Resource.h"

#include "Application.h"

#include <Windows.h>


extern AppState* app;
extern ResourceState* resource;

extern SDL_GPUCommandBuffer* cmdBuffer;


#define RESOURCE_FOLDER "res"
#define RESOURCE_PATH (PROJECT_PATH "/" RESOURCE_FOLDER "/")


void InitResourceState(ResourceState* resource)
{
	InitHashMap(&resource->modelNameMap);
	InitHashMap(&resource->textureNameMap);
	InitHashMap(&resource->fontDataNameMap);
	InitHashMap(&resource->fontNameMap);
	InitAnimationCache(&resource->animationCache);

	resource->directoryChangedHandle = FindFirstChangeNotificationA(PROJECT_PATH "/" RESOURCE_FOLDER, true, FILE_NOTIFY_CHANGE_LAST_WRITE);
}

FileWatcher* AddFileWatcher(const char* path)
{
	SDL_assert(resource->numFileWatchers < MAX_FILE_WATCHERS);

	SDL_PathInfo pathInfo = {};
	if (SDL_GetPathInfo(path, &pathInfo))
	{
		FileWatcher* watcher = &resource->fileWatchers[resource->numFileWatchers];
		SDL_strlcpy(watcher->path, path, sizeof(watcher->path));
		watcher->lastWriteTime = pathInfo.modify_time;
		resource->numFileWatchers++;
		return watcher;
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
		return nullptr;
	}
}

FileWatcher* GetFileWatcherFromPath(const char* path)
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

bool FileHasChanged(FileWatcher* file)
{
	//FileWatcher* watcher = GetFileWatcherFromPath(path);
	//SDL_assert(watcher);

	SDL_PathInfo pathInfo = {};
	if (SDL_GetPathInfo(file->path, &pathInfo))
	{
		if (pathInfo.modify_time > file->lastWriteTime)
		{
			file->lastWriteTime = pathInfo.modify_time;
			return true;
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
	}

	return false;
}

static void ResetFileChange(FileWatcher* file)
{
	SDL_PathInfo pathInfo = {};
	if (SDL_GetPathInfo(file->path, &pathInfo))
	{
		if (pathInfo.modify_time > file->lastWriteTime)
		{
			file->lastWriteTime = pathInfo.modify_time;
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
	}
}

static void AddHotReloadedResource(ResourceType type, const char* path, const char* path1, const char* path2, void* handle, void* handle1)
{
	SDL_assert(resource->numResourceWatchers < MAX_RESOURCE_WATCHERS);

	ResourceWatcher* watcher = &resource->resourceWatchers[resource->numResourceWatchers++];

	watcher->type = type;
	SDL_strlcpy(watcher->path, path, 256);

	char fullPath[256] = "";
	SDL_strlcpy(fullPath, RESOURCE_PATH, 256);
	SDL_strlcat(fullPath, path, 256);

	char fullPath1[256] = "";
	if (path1)
	{
		SDL_strlcpy(watcher->path1, path1, 256);

		SDL_strlcpy(fullPath1, RESOURCE_PATH, 256);
		SDL_strlcat(fullPath1, path1, 256);
	}

	char fullPath2[256] = "";
	if (path2)
	{
		SDL_strlcpy(watcher->path2, path2, 256);

		SDL_strlcpy(fullPath2, RESOURCE_PATH, 256);
		SDL_strlcat(fullPath2, path2, 256);
	}

	watcher->file = GetFileWatcherFromPath(fullPath);
	if (!watcher->file) watcher->file = AddFileWatcher(fullPath);

	watcher->file1 = path1 ? GetFileWatcherFromPath(fullPath1) : nullptr;
	if (!watcher->file1 && path1) watcher->file1 = AddFileWatcher(fullPath1);

	watcher->file2 = path2 ? GetFileWatcherFromPath(fullPath2) : nullptr;
	if (!watcher->file2 && path2) watcher->file2 = AddFileWatcher(fullPath2);

	watcher->handle = handle;
	watcher->handle1 = handle1;
}

void AddHotReloadedShader(const char* vertex, const char* fragment, const char* additionalPath, Shader* shader, GraphicsPipeline* pipeline)
{
	AddHotReloadedResource(RESOURCE_TYPE_GRAPHICS_SHADER, vertex, fragment, additionalPath, shader, pipeline);
}

void AddHotReloadedShader(const char* vertex, const char* fragment, Shader* shader, GraphicsPipeline* pipeline)
{
	AddHotReloadedResource(RESOURCE_TYPE_GRAPHICS_SHADER, vertex, fragment, nullptr, shader, pipeline);
}

void AddHotReloadedComputeShader(const char* path, const char* additionalPath, Shader* shader)
{
	AddHotReloadedResource(RESOURCE_TYPE_COMPUTE_SHADER, path, additionalPath, nullptr, shader, nullptr);
}

void UpdateHotReloadedResources()
{
	if (WaitForSingleObject(resource->directoryChangedHandle, 0) == WAIT_OBJECT_0)
	{
		FindNextChangeNotification(resource->directoryChangedHandle);

		for (int i = 0; i < resource->numResourceWatchers; i++)
		{
			ResourceWatcher* watcher = &resource->resourceWatchers[i];

			if (watcher->type == RESOURCE_TYPE_GRAPHICS_SHADER)
			{
				SDL_assert(watcher->file && watcher->file1 && watcher->handle && watcher->handle1);
				if (FileHasChanged(watcher->file) || FileHasChanged(watcher->file1) || watcher->file2 && FileHasChanged(watcher->file2))
				{
					Shader* shader = (Shader*)watcher->handle;
					GraphicsPipeline* pipeline = (GraphicsPipeline*)watcher->handle1;

					app->platformCallbacks.compileResources();

					char fullPath[256] = "";
					SDL_strlcpy(fullPath, RESOURCE_FOLDER "/", 256);
					SDL_strlcat(fullPath, watcher->path, 256);
					SDL_strlcat(fullPath, ".bin", 256);

					char fullPath1[256] = "";
					SDL_strlcpy(fullPath1, RESOURCE_FOLDER "/", 256);
					SDL_strlcat(fullPath1, watcher->path1, 256);
					SDL_strlcat(fullPath1, ".bin", 256);

					ReloadGraphicsShader(shader, fullPath, fullPath1);
					ReloadGraphicsPipeline(pipeline);
				}
			}
			else if (watcher->type == RESOURCE_TYPE_COMPUTE_SHADER)
			{
				SDL_assert(watcher->file && watcher->handle);
				if (FileHasChanged(watcher->file) || watcher->file1 && FileHasChanged(watcher->file1))
				{
					Shader* shader = (Shader*)watcher->handle;

					app->platformCallbacks.compileResources();

					char fullPath[256] = "";
					SDL_strlcpy(fullPath, RESOURCE_FOLDER "/", 256);
					SDL_strlcat(fullPath, watcher->path, 256);
					SDL_strlcat(fullPath, ".bin", 256);

					ReloadComputeShader(shader, fullPath);
				}
			}
			else
			{
				SDL_assert(false);
			}
		}

		for (int i = 0; i < resource->numFileWatchers; i++)
		{
			FileWatcher* watcher = &resource->fileWatchers[i];
			ResetFileChange(watcher);
		}
	}
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

Texture* GetTexture(const char* path)
{
	uint32_t pathHash = hash(path);
	if (int* textureID = HashMapGet(&resource->textureNameMap, pathHash))
		return resource->textures[*textureID];
	else
	{
		char fullPath[256];
		SDL_snprintf(fullPath, 256, "res/%s.bin", path);

		if (Texture* texture = LoadTexture(fullPath, cmdBuffer))
		{
			int textureID = resource->numTextures++;
			resource->textures[textureID] = texture;
			HashMapAdd(&resource->textureNameMap, pathHash, textureID);
			return texture;
		}

		return nullptr;
	}
}

FontData* LoadFont(const char* name, const char* path)
{
	uint32_t nameHash = hash(name);
	if (int* fontDataID = HashMapGet(&resource->fontNameMap, nameHash))
		return &resource->fontDatas[*fontDataID];
	else
	{
		char fullPath[256];
		SDL_snprintf(fullPath, 256, "res/%s.bin", path);

		{
			int fontDataID = resource->numFontDatas++;
			FontData* fontData = &resource->fontDatas[fontDataID];

			if (LoadFontData(fontData, fullPath))
			{
				HashMapAdd(&resource->fontDataNameMap, nameHash, fontDataID);
				return fontData;
			}
		}

		return nullptr;
	}
}

Font* GetFont(const char* name, float size)
{
	uint32_t nameHash = hash(name);
	if (int* fontDataID = HashMapGet(&resource->fontDataNameMap, nameHash))
	{
		FontData* fontData = &resource->fontDatas[*fontDataID];

		int fontID = resource->numFonts++;
		Font* font = &resource->fonts[fontID];

		InitFont(font, fontData, size);

		HashMapAdd(&resource->fontNameMap, nameHash, fontID);
		return font;
	}

	return nullptr;
}
