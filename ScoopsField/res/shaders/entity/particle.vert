#version 460

layout (location = 0) in vec2 a_vertexPosition;
layout (location = 1) in vec3 a_position;
layout (location = 2) in vec2 a_size;
layout (location = 3) in vec4 a_color;

layout (location = 0) out vec4 v_color;
layout (location = 1) out vec2 v_texcoord;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_viewModel;
	mat4 u_projection;
	mat4 u_model;
};


void main()
{
	vec4 position = vec4(a_position, 1);
	position += inverse(u_viewModel) * vec4(a_vertexPosition * a_size, 0, 1);

	gl_Position = u_projectionViewModel * position;

	v_color = a_color;
	v_texcoord = a_vertexPosition * vec2(1, -1) + 0.5;
}
