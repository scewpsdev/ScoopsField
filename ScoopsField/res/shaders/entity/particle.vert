#version 460

layout (location = 0) in vec2 a_vertexPosition;
layout (location = 1) in vec3 a_position;
layout (location = 2) in vec2 a_size;
layout (location = 3) in vec4 a_color;

layout (location = 0) out vec4 v_color;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_viewModel;
	mat4 u_projection;
	mat4 u_model;
};


void main()
{
	vec4 viewSpacePosition = u_viewModel * vec4(a_position, 1);
	viewSpacePosition.xy += a_vertexPosition * a_size;

	gl_Position = u_projection * viewSpacePosition;

	v_color = a_color;
}
