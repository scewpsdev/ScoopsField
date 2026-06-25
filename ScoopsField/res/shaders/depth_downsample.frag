#version 460

layout (location = 0) in vec2 v_texcoord;

layout (set = 2, binding = 0) uniform sampler2D s_input;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;

#define size params.xy
#define mip int(params.z + 0.5)
};


void main()
{
	vec2 texel = 1.0 / size;

	vec2 coord0 = v_texcoord - 0.25 * texel;
	vec2 coord1 = v_texcoord + 0.25 * vec2(texel.x, -texel.y);
	vec2 coord2 = v_texcoord + 0.25 * vec2(-texel.x, texel.y);
	vec2 coord3 = v_texcoord + 0.25 * texel;

	float depth0 = textureLod(s_input, coord0, mip - 1).r;
	float depth1 = textureLod(s_input, coord1, mip - 1).r;
	float depth2 = textureLod(s_input, coord2, mip - 1).r;
	float depth3 = textureLod(s_input, coord3, mip - 1).r;

	float result = max(max(depth0, depth1), max(depth2, depth2));

	gl_FragDepth = result;
}
