#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 i_position;
layout (location = 2) in vec3 i_color;

layout (location = 0) out vec2 v_texcoord;
layout (location = 1) out vec3 v_lightPosition;
layout (location = 2) out vec3 v_lightColor;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionView;
};


void main()
{
    vec3 worldPosition = a_position + i_position;

    gl_Position = u_projectionView * vec4(worldPosition, 1.0);

    v_texcoord = gl_Position.xy * vec2(1, -1) * 0.5 + 0.5;
    v_lightPosition = worldPosition;
    v_lightColor = i_color;
}