#version 460

layout (location = 0) in vec2 a_vertexPosition;
layout (location = 1) in vec3 a_position;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rotation;
layout (location = 4) in vec4 a_color;
layout (location = 5) in float a_animation;

layout (location = 0) out vec4 v_color;
layout (location = 1) out vec3 v_texcoord;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_viewModel;
	mat4 u_projection;
	mat4 u_model;
};


void main()
{
	vec2 vertexPosition = a_vertexPosition * a_size;
	float s = sin(a_rotation);
	float c = cos(a_rotation);
	vertexPosition = vec2(vertexPosition.x * c - vertexPosition.y * s, vertexPosition.x * s + vertexPosition.y * c);

	vec4 position = u_viewModel * vec4(a_position, 1);
	position.xy += vertexPosition;

	gl_Position = u_projection * position;

	v_color = a_color;
	v_texcoord = vec3(a_vertexPosition * vec2(1, -1) + 0.5, a_animation);
}
