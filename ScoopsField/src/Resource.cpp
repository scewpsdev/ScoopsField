#include "Resource.h"

#include "Application.h"


extern AppState* app;


void AddFileWatcher(const char* path)
{
	if (app->resourceState.numFileWatchers < MAX_FILE_WATCHERS)
	{
		FileWatcher* watcher = &app->resourceState.fileWatchers[app->resourceState.numFileWatchers];

		SDL_PathInfo pathInfo = {};
		if (SDL_GetPathInfo(path, &pathInfo))
		{
			SDL_strlcpy(watcher->path, path, sizeof(watcher->path));
			watcher->lastWriteTime = pathInfo.modify_time;

			app->resourceState.numFileWatchers++;
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
		}
	}
}

static FileWatcher* GetFileWatcherFromPath(const char* path)
{
	for (int i = 0; i < app->resourceState.numFileWatchers; i++)
	{
		if (SDL_strcmp(path, app->resourceState.fileWatchers[i].path) == 0)
		{
			return &app->resourceState.fileWatchers[i];
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
	return false;
}
