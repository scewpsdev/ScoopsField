#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_cloudCoverage;
layout(set = 2, binding = 1) uniform sampler3D s_cloudLowFrequency;
layout(set = 2, binding = 2) uniform sampler3D s_cloudHighFrequency;

layout(set = 2, binding = 3) uniform sampler2D s_bluenoise;
layout(set = 2, binding = 4) uniform sampler2D s_transmittanceLUT;
layout(set = 2, binding = 5) uniform sampler2D s_multiScatterLUT;
layout(set = 2, binding = 6) uniform sampler2D s_skyViewLUT;
layout(set = 2, binding = 7) uniform sampler3D s_cloudNoise;
layout(set = 2, binding = 8) uniform sampler3D s_cloudNoiseDetail;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 params2;
	mat4 projectionInv;
	mat4 viewInv;

	vec4 weatherData;

#define lightDirection params.xyz
#define gameTime params.w
#define frameIdx int(params2.w + 0.5)
#define cameraPosition params2.xyz
};


#include "../common.glsl"
#include "sky.glsl"
#include "clouds.glsl"


vec3 reconstructView(vec2 uv, mat4 projectionInv, mat4 viewInv)
{
	vec2 ndc = vec2(uv.x * 2 - 1, uv.y * -2 + 1);

	//float aspect = projection[1][1] / projection[0][0];
	//ndc.x *= aspect;

	//float tanHalfFov = 1.0 / projection[1][1]; //tan(fov * 0.5);

	vec3 dir;
	dir.x = ndc.x * projectionInv[0][0];
	dir.y = ndc.y * projectionInv[1][1];
	dir.z = -1;

	dir = mat3(viewInv) * dir;
	dir = normalize(dir);

	return dir;
}

vec3 sampleSkyViewLUT(vec3 dir)
{
	float longitude = mod(atan(dir.x, dir.z) + 2 * pi, 2 * pi);
	float latitude = asin(dir.y);

	float u = longitude / pi * 0.5;
	float v = 0.5 + 0.5 * -sign(latitude) * sqrt(abs(latitude) / pi * 2);

	vec3 color = texture(s_skyViewLUT, vec2(u, v)).rgb;

	return color;
}

void main()
{
	vec3 dir = reconstructView(v_texcoord, projectionInv, viewInv); // view space direction

	vec3 color = sampleSkyViewLUT(dir);

	vec4 cloudColor = clouds(cameraPosition, dir, lightDirection, 0, 2, 8);
	color = mix(cloudColor.rgb, color, cloudColor.a);

	out_color = vec4(color, 1);
}
