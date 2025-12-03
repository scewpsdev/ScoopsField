#version 460

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_color;

layout (location = 0) out vec4 out_normal;
layout (location = 1) out vec3 out_color;


void main()
{
	out_normal = vec4(v_normal, 1);
	out_color = v_color;
}
