#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_depth;

layout(set = 2, binding = 1) uniform sampler2D s_cloudCoverage;
layout(set = 2, binding = 2) uniform sampler3D s_cloudLowFrequency;
layout(set = 2, binding = 3) uniform sampler3D s_cloudHighFrequency;

layout(set = 2, binding = 4) uniform sampler2D s_bluenoise;
layout(set = 2, binding = 5) uniform sampler2D s_lastFrame;
layout(set = 2, binding = 6) uniform sampler2D s_transmittanceLUT;
layout(set = 2, binding = 7) uniform sampler2D s_multiScatterLUT;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 params2;
	mat4 projectionInv;
	mat4 viewInv;
	mat4 lastProjection;
	mat4 lastView;

#define lightDirection params.xyz
#define gameTime params.w
#define frameIdx int(params2.x + 0.5)
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

vec2 reconstructUV(vec3 dir, mat4 projection, mat4 view)
{
	dir = normalize(mat3(view) * dir);

	vec4 clip = projection * vec4(dir, 1);
	vec2 ndc = clip.xy / clip.w;
	vec2 uv = vec2(ndc.x * 0.5 + 0.5, ndc.y * -0.5 + 0.5);
	
	return uv;
}

float bluenoise(vec2 coord)
{
	vec2 size = textureSize(s_bluenoise, 0);
	vec2 uv = coord / size;
	return texture(s_bluenoise, uv).r;
}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth != 0)
	{
		out_color = vec4(0);
		return;
	}

	vec3 view = reconstructView(v_texcoord, projectionInv, viewInv); // view space direction

	SkySettings sky;
	sky.numSamples = 16;
	sky.numLightSamples = 4;
	sky.offsetRayStart = true;
	sky.noise = fract(bluenoise(gl_FragCoord.xy) + frameIdx * 0.61803398875);
	sky.lod = false;
	sky.time = gameTime;

	vec3 color = atmosphere(view, lightDirection, sky);

	vec4 cloudColor = clouds(view, lightDirection, sky);
	color = mix(color, cloudColor.rgb, cloudColor.a);

	// accumulation
	vec2 lastUV = reconstructUV(view, lastProjection, lastView);
	if (lastUV.x >= 0 && lastUV.x <= 1 && lastUV.y >= 0 && lastUV.y <= 1)
	{
		float lastDepth = texture(s_depth, lastUV).r;
		if (lastDepth == 0)
		{
			vec4 lastColor = texture(s_lastFrame, lastUV);
			if (lastColor.a == 1)
				color = mix(color, lastColor.rgb, 0.9);
		}
	}

	out_color = vec4(color, 1);
}
