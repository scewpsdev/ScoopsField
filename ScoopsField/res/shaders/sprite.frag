#version 460

layout (location = 0) in vec2 v_texcoord;
layout (location = 1) in vec4 v_color;

layout (location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D u_texture;


void main()
{
    vec4 textureColor = texture(u_texture, v_texcoord);
    FragColor = v_color * textureColor;
}