#version 460

#include "common.shader"

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec3 v_color;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_model;
};


void main()
{
	gl_Position = u_projectionViewModel * vec4(a_position, 1);

	vec4 worldNormal = u_model * vec4(a_normal, 0);

	v_normal = worldNormal.xyz;
	v_color = SRGBToLinear(vec3(0.5));
}
