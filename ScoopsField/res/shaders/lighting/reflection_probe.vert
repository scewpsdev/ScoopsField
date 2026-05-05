#version 460

layout (location = 0) in vec3 a_position;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
	mat4 projectionView;
	vec4 params;
	vec4 params2;

#define probePosition params.xyz
#define probeSize params2.xyz
};


void main()
{
	vec3 worldPosition = a_position * (probeSize + 1) + probePosition;

	gl_Position = projectionView * vec4(worldPosition, 1.0);
}