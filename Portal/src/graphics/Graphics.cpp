#include "Graphics.h"


void InitGraphics(GraphicsState* graphics)
{
	InitPool(&graphics->vertexBuffers);
	InitPool(&graphics->indexBuffers);
	InitPool(&graphics->indirectBuffers);
	InitPool(&graphics->storageBuffers);
	InitPool(&graphics->transferBuffers);
	InitPool(&graphics->shaders);
	InitPool(&graphics->textures);
	InitPool(&graphics->renderTargets);
	InitPool(&graphics->graphicsPipelines);
}
