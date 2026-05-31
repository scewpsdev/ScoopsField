#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texcoord;
layout (location = 2) in vec4 a_color;

layout (location = 0) out vec2 v_texcoord;
layout (location = 1) out vec4 v_color;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_model;
};


void main()
{
	gl_Position = u_projectionViewModel * vec4(a_position, 1);

	v_texcoord = a_texcoord;
	v_color = a_color;
}
