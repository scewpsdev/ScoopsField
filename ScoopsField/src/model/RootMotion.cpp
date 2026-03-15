#include "RootMotion.h"	

#include "Model.h"
#include "Animation.h"


mat4 DoRootMotion(AnimationState* anim, Node* rootNode, mat4* lastRootNodeTransform)
{
	mat4 rootNodeTransform = GetNodeTransform(anim, rootNode);
	vec3 rootMotion = rootNodeTransform.translation();
	SDL_Log("%.2f, %.2f, %.2f\n", rootMotion.x, rootMotion.y, rootMotion.z);

	mat4 delta = rootNodeTransform * lastRootNodeTransform->inverted();
	*lastRootNodeTransform = rootNodeTransform;
	rootNodeTransform = mat4::Identity;
	return delta;
}
