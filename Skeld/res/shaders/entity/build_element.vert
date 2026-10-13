#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 4) in vec2 a_texcoord;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec2 v_texcoord;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_view;
	mat4 u_projection;
	mat4 u_model;
	vec4 params;

#define u_time params.x
#define viewSpaceBuffer params.y
};


void main()
{
	vec4 worldPosition = u_model * vec4(a_position, 1);
	gl_Position = u_projection * u_view * worldPosition;

	vec4 viewSpaceNormal = u_model * vec4(a_normal, 0);
	if (viewSpaceBuffer > 0.5) viewSpaceNormal = u_view * viewSpaceNormal;
	v_normal = viewSpaceNormal.xyz;

	vec2 texcoord = vec2(worldPosition.x, -worldPosition.y);
	texcoord += a_normal.y > 0.5 ? vec2(0, worldPosition.z) : vec2(worldPosition.z, 0);
	texcoord *= 0.15;

	v_texcoord = texcoord;
}
