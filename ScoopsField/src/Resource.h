#pragma once

#include "graphics/Shader.h"

#include "model/Model.h"
#include "model/AnimationCache.h"

#include "utils/StringView.h"

#include <SDL3/SDL.h>


struct FileWatcher
{
	char path[256];
	int64_t lastWriteTime;
};

struct ResourceState
{
	AnimationCache animationCache;

#define MAX_FILE_WATCHERS 16
	FileWatcher fileWatchers[MAX_FILE_WATCHERS];
	int numFileWatchers;
};


void AddFileWatcher(const char* path);
bool FileHasChanged(const char* path);

StringView GetDirectory(const char* path);
void GetAbsolutePath(char* str, int maxLen, const char* relativePath, const char* relativeTo);
void GetRelativePath(char* str, int maxLen, const char* absolutePath, const char* relativeTo);
