#version 460

#include "../common.glsl"

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec3 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_texture;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 materialData0;
	vec4 data1;
	vec4 data2;
	vec4 data3;
	vec4 params;

#define hasTexture materialData0.x
#define hasAnimation materialData0.y
#define atlasSize ivec2(materialData0.zw + 0.5)

#define cameraPosition params.xyz
};


void main()
{
	vec2 uv = v_texcoord.xy;
	float animation = v_texcoord.z;

	float frameIdx = max(animation * atlasSize.x * atlasSize.y - 1, 0.0);

	int frameX = int(frameIdx) % atlasSize.x;
	int frameY = int(frameIdx) / atlasSize.x;
	vec2 frameUV = (uv + vec2(frameX, frameY)) / atlasSize;
	vec4 frameColor = SRGBToLinear(texture(s_texture, frameUV));

	int nextFrameX = int(frameIdx + 1) % atlasSize.x;
	int nextFrameY = int(frameIdx + 1) / atlasSize.x;
	vec2 nextFrameUV = (uv + vec2(nextFrameX, nextFrameY)) / atlasSize;
	vec4 nextFrameColor = SRGBToLinear(texture(s_texture, nextFrameUV));

	float blend = fract(frameIdx);
	vec4 animationColor = mix(frameColor, nextFrameColor, blend);

	vec4 textureColor = mix(vec4(1), animationColor, hasTexture);
	vec4 color = v_color * textureColor;

	if (color.a < 0.0001)
		discard;
	
	out_color = color;
}
