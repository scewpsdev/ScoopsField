#pragma once

#include "graphics/Shader.h"
#include "graphics/Font.h"

#include "model/Model.h"
#include "model/AnimationCache.h"

#include "utils/StringView.h"
#include "utils/HashMap.h"

#include <SDL3/SDL.h>


struct FileWatcher
{
	char path[256];
	int64_t lastWriteTime;
};

enum ResourceType
{
	RESOURCE_TYPE_NULL = 0,

	RESOURCE_TYPE_GRAPHICS_SHADER,
	RESOURCE_TYPE_COMPUTE_SHADER,
};

struct ResourceWatcher
{
	ResourceType type;
	char path[256];
	char path1[256];
	FileWatcher* file, * file1;
	void* handle, * handle1;
};

struct ResourceState
{
#define MAX_MODEL_RESOURCES 64
	Model models[MAX_MODEL_RESOURCES];
	HashMap<uint32_t, int, MAX_MODEL_RESOURCES> modelNameMap;
	int numModels;

#define MAX_TEXTURE_RESOURCES 64
	Texture* textures[MAX_TEXTURE_RESOURCES];
	HashMap<uint32_t, int, MAX_TEXTURE_RESOURCES> textureNameMap;
	int numTextures;

#define MAX_FONT_DATA_RESOURCES 8
	FontData fontDatas[MAX_FONT_DATA_RESOURCES];
	HashMap<uint32_t, int, MAX_FONT_DATA_RESOURCES> fontDataNameMap;
	int numFontDatas;

#define MAX_FONT_RESOURCES 16
	Font fonts[MAX_FONT_RESOURCES];
	HashMap<uint32_t, int, MAX_FONT_RESOURCES> fontNameMap;
	int numFonts;

	AnimationCache animationCache;

#define MAX_FILE_WATCHERS 64
	FileWatcher fileWatchers[MAX_FILE_WATCHERS];
	int numFileWatchers;

	void* directoryChangedHandle;
#define MAX_RESOURCE_WATCHERS 32
	ResourceWatcher resourceWatchers[MAX_RESOURCE_WATCHERS];
	int numResourceWatchers;
};

struct Shader;
struct GraphicsPipeline;


void InitResourceState(ResourceState* resource);

FileWatcher* AddFileWatcher(const char* path);
bool FileHasChanged(FileWatcher* file);
FileWatcher* GetFileWatcherFromPath(const char* path);

void AddHotReloadedShader(const char* vertex, const char* fragment, Shader* shader, GraphicsPipeline* pipeline);
void AddHotReloadedComputeShader(const char* path, const char* path1, Shader* shader);
void UpdateHotReloadedResources();

StringView GetDirectory(const char* path);
void GetAbsolutePath(char* str, int maxLen, const char* relativePath, const char* relativeTo);
void GetRelativePath(char* str, int maxLen, const char* absolutePath, const char* relativeTo);

Model* GetModel(const char* path);
Texture* GetTexture(const char* path);

FontData* LoadFont(const char* name, const char* path);
Font* GetFont(const char* name, float size);
