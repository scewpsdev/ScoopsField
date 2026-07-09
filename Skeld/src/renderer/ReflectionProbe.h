#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"

#include <SDL3/SDL.h>


#define REFLECTION_PROBE_RESOLUTION 128


struct RenderTarget;

struct ReflectionProbe
{
	vec3 position;
	vec3 size;

	SDL_GPUBuffer* irradiance;
	SDL_GPUTexture* specular;
};


void InitReflectionProbe(ReflectionProbe* probe, vec3 position, vec3 size);
void DestroyReflectionProbe(ReflectionProbe* probe);
