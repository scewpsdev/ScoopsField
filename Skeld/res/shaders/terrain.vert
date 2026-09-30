#version 460

layout (location = 0) in float a_height;
layout (location = 1) in vec3 a_normal;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec2 v_texcoord;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_view;
	mat4 u_projection;
	mat4 u_model;
};

#define TILE_SIZE 2.0
#define VERTICES 129


void main()
{
	float x = (gl_VertexIndex % VERTICES) * TILE_SIZE;
	float z = (gl_VertexIndex / VERTICES) * TILE_SIZE;
	vec3 position = vec3(x, a_height, z);

	gl_Position = u_projectionViewModel * vec4(position, 1);

	vec4 viewSpaceNormal = u_model * vec4(a_normal, 0);

	vec2 texcoord = position.xz / 5;

	v_normal = viewSpaceNormal.xyz;
	v_texcoord = texcoord;
}
