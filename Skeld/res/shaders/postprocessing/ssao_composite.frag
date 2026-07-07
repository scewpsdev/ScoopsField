#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_texture;


void main()
{
    float ao = texture(s_texture, v_texcoord).r;
    out_color = vec4(ao, ao, ao, 1);
}