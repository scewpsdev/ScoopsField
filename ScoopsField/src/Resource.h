#pragma once

#include "graphics/Shader.h"

#include "model/Mesh.h"

#include <SDL3/SDL.h>


struct FileWatcher
{
	char path[256];
	int64_t lastWriteTime;
};

struct ResourceState
{
#define MAX_MESHES 256
	Mesh meshes[MAX_MESHES];
	int numMeshes;

#define MAX_FILE_WATCHERS 16
	FileWatcher fileWatchers[MAX_FILE_WATCHERS];
	int numFileWatchers;
};


void AddFileWatcher(const char* path);
bool FileHasChanged(const char* path);
