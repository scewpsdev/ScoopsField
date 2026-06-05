#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"

#include <PxPhysics.h>


struct Articulation
{
	physx::PxArticulationReducedCoordinate* articulation;
};


void InitArticulation(Articulation* articulation);
void DestroyArticulation(Articulation* articulation);
void SpawnArticulation(Articulation* articulation);
void SetArticulationRootTransform(Articulation* articulation, vec3 position, quat rotation);
void SetArticulationRootVelocity(Articulation* articulation, vec3 linearVelocity, vec3 angularVelocity);
