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
	vec4 u_clippingPlane;
};


void main()
{
	vec4 viewSpacePosition = u_model * vec4(a_position, 1);
	gl_Position = u_projection * viewSpacePosition;
	//gl_Position = u_projectionViewModel * vec4(a_position, 1);

	gl_ClipDistance[0] = dot(viewSpacePosition, u_clippingPlane);

	vec4 viewSpaceNormal = u_model * vec4(a_normal, 0);

	v_normal = viewSpaceNormal.xyz;
	v_texcoord = a_texcoord;
}
