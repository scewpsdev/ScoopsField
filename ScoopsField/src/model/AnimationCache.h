#pragma once

#include "model/Model.h"

#include "utils/BumpAllocator.h"


struct AnimationCache
{
#define MAX_POSITION_KEYFRAMES 1024
	PositionKeyframe positions[MAX_POSITION_KEYFRAMES];
#define MAX_ROTATION_KEYFRAMES 8192
	RotationKeyframe rotations[MAX_ROTATION_KEYFRAMES];
#define MAX_SCALING_KEYFRAMES 1024
	ScalingKeyframe scalings[MAX_SCALING_KEYFRAMES];

	BumpAllocator positionAllocator;
	BumpAllocator rotationAllocator;
	BumpAllocator scalingAllocator;
};


void InitAnimationCache(AnimationCache* animationCache);
