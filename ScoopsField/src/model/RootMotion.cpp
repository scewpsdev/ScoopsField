#include "RootMotion.h"	

#include "Model.h"
#include "Animation.h"


mat4 DoRootMotion(AnimationState* anim, Node* rootNode, mat4* lastRootNodeTransform)
{
	mat4 rootNodeTransform = GetNodeTransform(anim, rootNode);
	mat4 delta = rootNodeTransform * lastRootNodeTransform->inverted();
	*lastRootNodeTransform = rootNodeTransform;
	rootNodeTransform = mat4::Identity;
	return delta;
}
