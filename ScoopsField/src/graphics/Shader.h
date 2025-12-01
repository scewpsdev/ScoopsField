#pragma once

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>


struct Shader
{
	SDL_GPUShader* vertex;
	SDL_GPUShader* fragment;
	SDL_GPUComputePipeline* compute;
};


Shader* LoadGraphicsShader(const char* vertexPath, const char* fragmentPath);
Shader* LoadComputeShader(const char* computePath);
void DestroyShader(Shader* shader);

void ReloadGraphicsShader(Shader* shader, const char* vertexPath, const char* fragmentPath);
