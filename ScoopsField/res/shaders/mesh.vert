#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec3 v_color;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
};


void main()
{
	gl_Position = u_projectionViewModel * vec4(a_position, 1);

	v_normal = a_normal;
	v_color = vec3(0.5);
}
