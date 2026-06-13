#version 460

#include "../common.glsl"
#include "../lighting/lighting.glsl"

layout (location = 0) in vec2 v_texcoord;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec3 v_position;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_texture;
layout(set = 2, binding = 1) uniform samplerCube s_environment;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 params1;
	vec4 pointLightPositions[4];
	vec4 pointLightColors[4];

#define hasTexture params.x
#define emissive params.y

#define cameraPosition params1.xyz
#define numPointLights int(params1.w + 0.5)
};


vec3 lighting(vec3 position)
{
	vec3 radiance = vec3(0);
	for (int i = 0; i < numPointLights; i++)
	{
		vec3 toLight = pointLightPositions[i].xyz - position;
		float distanceSq = dot(toLight, toLight);
		radiance += L(pointLightColors[i].rgb, distanceSq, 0.1) / PI;
	}
	return radiance;
}

void main()
{
	vec4 textureColor = mix(vec4(1), texture(s_texture, v_texcoord), hasTexture);
	vec4 color = textureColor * v_color;

	vec3 radiance = color.rgb * (emissive + lighting(v_position));
	radiance += color.rgb * textureLod(s_environment, vec3(0, 1, 0), 12).rgb;

	out_color = vec4(radiance, color.a);
}
