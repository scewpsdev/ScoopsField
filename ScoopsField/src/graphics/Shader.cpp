#include "Shader.h"

#include "Application.h"

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


void DestroyShader(Shader* shader)
{
	if (shader->compute)
		SDL_ReleaseGPUComputePipeline(device, shader->compute);
	else
	{
		SDL_ReleaseGPUShader(device, shader->vertex);
		SDL_ReleaseGPUShader(device, shader->fragment);
	}
}

static SDL_GPUShader* LoadGraphicsShaderStage(const char* path, SDL_ShaderCross_ShaderStage stage)
{
	size_t codeSize;
	void* code = SDL_LoadFile(path, &codeSize);

	if (code)
	{
		bool gpuDebug = false;
#if _DEBUG
		gpuDebug = true;
#endif

		SDL_ShaderCross_SPIRV_Info shaderInfo = {};
		shaderInfo.bytecode = (Uint8*)code;
		shaderInfo.bytecode_size = codeSize;
		shaderInfo.entrypoint = "main";
		shaderInfo.shader_stage = stage;
		shaderInfo.enable_debug = gpuDebug;

		SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV((Uint8*)code, codeSize, 0);
		SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &shaderInfo, metadata, 0);

		SDL_free(metadata);
		SDL_free(code);

		return shader;
	}

	return nullptr;
}

Shader* LoadGraphicsShader(const char* vertexPath, const char* fragmentPath)
{
	SDL_GPUShader* vertex = LoadGraphicsShaderStage(vertexPath, SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
	SDL_GPUShader* fragment = LoadGraphicsShaderStage(fragmentPath, SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);

	SDL_assert(graphics->numShaders < MAX_SHADERS);

	Shader* shader = &graphics->shaders[graphics->numShaders++];
	shader->vertex = vertex;
	shader->fragment = fragment;

	return shader;
}

void ReloadGraphicsShader(Shader* shader, const char* vertexPath, const char* fragmentPath)
{
	SDL_ReleaseGPUShader(device, shader->vertex);
	SDL_ReleaseGPUShader(device, shader->fragment);

	shader->vertex = LoadGraphicsShaderStage(vertexPath, SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
	shader->fragment = LoadGraphicsShaderStage(fragmentPath, SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
}

static SDL_GPUComputePipeline* LoadComputeShaderStage(const char* path)
{
	size_t codeSize;
	void* code = SDL_LoadFile(path, &codeSize);

	if (code)
	{
		SDL_ShaderCross_SPIRV_Info shaderInfo = {};
		shaderInfo.bytecode = (Uint8*)code;
		shaderInfo.bytecode_size = codeSize;
		shaderInfo.entrypoint = "main";
		shaderInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;

		SDL_ShaderCross_ComputePipelineMetadata* metadata = SDL_ShaderCross_ReflectComputeSPIRV((Uint8*)code, codeSize, 0);
		SDL_GPUComputePipeline* shader = SDL_ShaderCross_CompileComputePipelineFromSPIRV(device, &shaderInfo, metadata, 0);

		SDL_free(metadata);
		SDL_free(code);

		return shader;
	}

	return nullptr;
}

Shader* LoadComputeShader(const char* computePath)
{
	SDL_GPUComputePipeline* compute = LoadComputeShaderStage(computePath);

	SDL_assert(graphics->numShaders < MAX_SHADERS);

	Shader* shader = &graphics->shaders[graphics->numShaders++];
	shader->compute = compute;

	return shader;
}
