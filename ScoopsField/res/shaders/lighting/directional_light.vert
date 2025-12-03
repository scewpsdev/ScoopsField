#version 460

layout (location = 0) in vec3 a_position;

layout (location = 0) out vec2 v_texcoord;


void main()
{
    gl_Position = vec4(a_position, 1.0);

    v_texcoord = a_position.xy * vec2(1, -1) * 0.5 + 0.5;
}