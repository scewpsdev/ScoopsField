#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 i_position;
layout (location = 2) in vec4 i_color;

layout (location = 0) out vec3 v_lightPosition;
layout (location = 1) out vec3 v_lightColor;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionView;
};


void main()
{
    vec3 worldPosition = a_position * i_position.w + i_position.xyz;

    gl_Position = u_projectionView * vec4(worldPosition, 1.0);

    v_lightPosition = i_position.xyz;
    v_lightColor = i_color.rgb;
}