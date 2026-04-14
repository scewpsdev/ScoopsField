#version 460

#include "common.glsl"

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_cloudCoverage;
layout(set = 2, binding = 1) uniform sampler3D s_cloudLowFrequency;
layout(set = 2, binding = 2) uniform sampler3D s_cloudHighFrequency;

layout(set = 2, binding = 3) uniform sampler2D s_bluenoise;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 params2;
	mat4 projectionInv;
	mat4 viewInv;

#define lightDirection params.xyz
#define time params.w
#define frameIdx int(params2.x + 0.5)
};


#include "sky.glsl"


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

void main()
{
	vec3 view = reconstructView(v_texcoord, projectionInv, viewInv); // view space direction

	SkySettings settings;
	settings.numSamples = 16;
	settings.numLightSamples = 6;
	settings.bluenoise = false;
	settings.lod = true;

	vec3 color = atmosphere(view, lightDirection, settings);

	out_color = vec4(color, 1);
}
