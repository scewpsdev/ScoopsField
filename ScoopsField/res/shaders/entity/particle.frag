#version 460

#include "../common.glsl"
#include "../lighting/lighting.glsl"

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec3 v_texcoord;
layout (location = 2) in vec3 v_position;
layout (location = 3) in vec3 v_normal;

layout (location = 0) out vec4 out_color;

layout (set = 2, binding = 0) uniform sampler2D s_texture;
layout (set = 2, binding = 1) uniform samplerCube s_environment;

layout (set = 3, binding = 0) uniform UniformBlock {
	vec4 data0;
	vec4 data1;

	vec4 params;
	vec4 pointLightPositions[4];
	vec4 pointLightColors[4];

#define hasTexture data0.x
#define hasAnimation data0.y
#define atlasSize ivec2(data0.zw + 0.5)
#define emissive data1.x

#define cameraPosition params.xyz
#define numPointLights int(params.w + 0.5)
};


vec3 lighting(vec3 position, vec3 normal)
{
	vec3 radiance = vec3(0);
	for (int i = 0; i < numPointLights; i++)
	{
		vec3 toLight = pointLightPositions[i].xyz - position;
		float distanceSq = dot(toLight, toLight);
		float ndotwi = max(dot(toLight, normal), 0.0);
		radiance += L(pointLightColors[i].rgb, distanceSq, 0.1) * ndotwi / PI;
	}
	radiance += textureLod(s_environment, normal, 12).rgb;
	return radiance;
}

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

	vec3 radiance = color.rgb * (emissive + lighting(v_position, v_normal));

	if (color.a < 0.0001)
		discard;
	
	out_color = vec4(radiance, color.a);
}
