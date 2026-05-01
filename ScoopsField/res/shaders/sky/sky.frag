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
layout(set = 2, binding = 8) uniform sampler2D s_skyViewLUT;
layout(set = 2, binding = 9) uniform sampler3D s_cloudNoise;
layout(set = 2, binding = 10) uniform sampler3D s_cloudNoiseDetail;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 params2;
	mat4 projectionInv;
	mat4 viewInv;
	mat4 lastProjection;
	mat4 lastView;

	vec4 weatherData;

#define lightDirection params.xyz
#define gameTime params.w
#define frameIdx int(params2.x + 0.5)
};


#include "../common.glsl"
#include "sky.glsl"
#include "clouds.glsl"


vec3 reconstructRay(vec2 uv, mat4 projectionInv)
{
	vec2 ndc = vec2(uv.x * 2 - 1, uv.y * -2 + 1);

	/*
	vec4 viewSpacePosition = projectionInv * vec4(ndc, 1, 1);
	viewSpacePosition.xyz /= viewSpacePosition.w;

	return normalize(viewSpacePosition.xyz);
	*/

	//float aspect = projection[1][1] / projection[0][0];
	//ndc.x *= aspect;

	//float tanHalfFov = 1.0 / projection[1][1]; //tan(fov * 0.5);

	vec3 dir;
	dir.x = ndc.x * projectionInv[0][0];
	dir.y = ndc.y * projectionInv[1][1];
	dir.z = -1;

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

float reconstructDistance(float depth, vec3 viewRay)
{
	//float near = 1.0 / projectionInv[2][3];
	//float viewZ = near / depth;
	float viewZ = -1.0 / (depth * projectionInv[2][3] + projectionInv[3][3]);
	float dist = viewZ / viewRay.z;
	return dist;
}

float bluenoise(vec2 coord)
{
	vec2 size = textureSize(s_bluenoise, 0);
	vec2 uv = coord / size;
	return texture(s_bluenoise, uv).r;
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
	vec3 viewRay = reconstructRay(v_texcoord, projectionInv); // view space direction
	vec3 dir = mat3(viewInv) * viewRay;
	vec3 cameraPosition = viewInv[3].xyz;

	float depth = texture(s_depth, v_texcoord).r;
	if (depth != 0)
	{
		float maxDistance = reconstructDistance(depth, viewRay);
		out_color = calculateAerial(cameraPosition, dir, maxDistance, lightDirection);
	}
	else
	{
		vec3 color = sampleSkyViewLUT(dir);

		vec3 pos = cameraPosition + vec3(0, planetRadius, 0);
		float height = length(pos);
		vec3 up = pos / height;
		height -= planetRadius;

		// sun
		float sunIntensity = 25;
		vec3 sunlight = sampleTransmittanceLUT(height, vec3(-lightDirection.x, max(-lightDirection.y, 0), -lightDirection.z), up) * sunIntensity * 10;
		float sunSize = mix(0.0004, 0.0002, max(dot(-lightDirection, vec3(0, 1, 0)), 0));
		float sunAlpha = smoothstep(1 - sunSize, 1.0, dot(dir, -lightDirection)) * smoothstep(-0.005, 0.002, dir.y);
		color = mix(color, sunlight, sunAlpha);

		// clouds
		float noise = fract(bluenoise(gl_FragCoord.xy) + frameIdx * 0.61803398875) - 0.5;
		vec4 cloudColor = clouds(cameraPosition, dir, lightDirection, noise);
		color = mix(cloudColor.rgb, color, cloudColor.a);

		// temporal accumulation
		vec2 lastUV = reconstructUV(dir, lastProjection, lastView);
		if (lastUV.x >= 0 && lastUV.x <= 1 && lastUV.y >= 0 && lastUV.y <= 1)
		{
			float lastDepth = texture(s_depth, lastUV).r;
			if (lastDepth == 0)
			{
				vec4 lastColor = texture(s_lastFrame, lastUV);
				if (lastColor.a == 0)
					color = mix(color, lastColor.rgb, 0.9);
			}
		}

		out_color = vec4(color, 0);
	}
}
