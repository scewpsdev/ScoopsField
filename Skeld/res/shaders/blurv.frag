#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_texture;
layout(set = 2, binding = 1) uniform sampler2D s_depth;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;

#define near params.x
};


void getSample(vec2 uv, float depth, vec2 texel, inout vec4 value, inout float sum)
{
	vec2 snappedUv = uv / texel;
	snappedUv = floor(snappedUv) + 0.25;
	snappedUv *= texel;

	float sampleDepth = texture(s_depth, snappedUv).r;

	float adjustedDepthDistance = abs(near / sampleDepth - near / depth);
	float adjustedEpsilon = near / depth * 0.03;

	float weight = sampleDepth > 0 && adjustedDepthDistance < adjustedEpsilon ? 1 : 0;
	value += texture(s_texture, uv) * weight;
	sum += weight;
}

void main()
{
	vec2 texel = 1.0 / textureSize(s_texture, 0);

	vec2 snappedUv = v_texcoord / texel;
	snappedUv = floor(snappedUv) + 0.25;
	snappedUv *= texel;

	float depth = texture(s_depth, snappedUv).r;
	if (depth == 0)
		discard;

	vec4 color = vec4(0);
	float sum = 0;
	int kernelSize = 7;

	for (int i = 0; i < kernelSize; i++)
	{
		float offset = (i - kernelSize / 2) * texel.y;
		getSample(v_texcoord + vec2(0, offset), depth, texel, color, sum);
	}

	if (sum > 0)
		color /= sum;
		
	out_color = color;
}
