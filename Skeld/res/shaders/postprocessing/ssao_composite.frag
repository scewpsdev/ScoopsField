#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout (set = 2, binding = 0) uniform sampler2D s_texture;

layout (set = 3, binding = 0) uniform UniformBlock {
	vec4 params;

#define lod int(params.x + 0.5)
};


void main()
{
    float ao = textureLod(s_texture, v_texcoord, lod).r;
    out_color = vec4(ao, ao, ao, 1);
}