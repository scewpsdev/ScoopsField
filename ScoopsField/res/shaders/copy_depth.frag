#version 460

layout (location = 0) in vec2 v_texcoord;

layout (set = 2, binding = 0) uniform sampler2D s_depth;


void main()
{
	gl_FragDepth = texture(s_depth, v_texcoord).r;
}
