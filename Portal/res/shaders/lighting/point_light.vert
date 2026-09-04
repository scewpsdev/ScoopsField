#version 460

layout (location = 0) in vec3 a_position;
//layout (location = 1) in vec4 i_position;
//layout (location = 2) in vec4 i_color;

layout (location = 0) out vec3 v_lightPosition;
layout (location = 1) out vec3 v_lightColor;
layout (location = 2) out vec4 v_maskPlanes[5];

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 projectionView;
    vec4 maskPlanes[5];
    vec3 lightPosition;
    float lightRadius;
    vec4 lightColor;
};


void main()
{
    vec3 worldPosition = a_position * lightRadius + lightPosition;

    gl_Position = projectionView * vec4(worldPosition, 1.0);

    v_lightPosition = lightPosition;
    v_lightColor = lightColor.rgb;

    for (int i = 0; i < 5; i++)
        v_maskPlanes[i] = maskPlanes[i];
}