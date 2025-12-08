#include "AnimationCache.h"


void InitAnimationCache(AnimationCache* animationCache)
{
	InitBumpAllocator(&animationCache->positionAllocator, (uint8_t*)animationCache->positions, sizeof(animationCache->positions));
	InitBumpAllocator(&animationCache->rotationAllocator, (uint8_t*)animationCache->rotations, sizeof(animationCache->rotations));
	InitBumpAllocator(&animationCache->scalingAllocator, (uint8_t*)animationCache->scalings, sizeof(animationCache->scalings));
}
