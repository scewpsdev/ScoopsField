#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 4) in vec2 a_texcoord;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec2 v_texcoord;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_model;
};


void main()
{
	gl_Position = u_projectionViewModel * vec4(a_position, 1);

	vec4 worldNormal = u_model * vec4(a_normal, 0);

	v_normal = worldNormal.xyz;
	v_texcoord = a_texcoord;
}
