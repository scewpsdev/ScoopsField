#pragma once

#include "graphics/Shader.h"

#include <SDL3/SDL.h>


struct FileWatcher
{
	char path[256];
	int64_t lastWriteTime;
};

struct ResourceState
{
#define MAX_FILE_WATCHERS 16
	int numFileWatchers;
	FileWatcher fileWatchers[MAX_FILE_WATCHERS];
};


void AddFileWatcher(const char* path);
bool FileHasChanged(const char* path);
