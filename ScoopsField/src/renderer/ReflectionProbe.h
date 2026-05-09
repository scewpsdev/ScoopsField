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

	RenderTarget* cubemap;
	SDL_GPUBuffer* irradiance;
};


void InitReflectionProbe(ReflectionProbe* probe, vec3 position, vec3 size);
