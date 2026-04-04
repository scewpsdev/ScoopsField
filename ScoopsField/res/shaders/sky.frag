#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	mat4 projectionInv;
	mat4 viewInv;

#define lightDirection params.xyz
};


#define pi 3.14159265359

#define planetRadius 6360e3
#define atmosphereRadius 6420e3
#define rayleighScatter vec3(3.8e-6f, 13.5e-6f, 33.1e-6f)
#define mieScatter 21e-6f
#define rayleighHeightScale 7994
#define mieHeightScale 1200
#define mieAnisotropy 0.76

#define numSamples 16
#define numLightSamples 8


bool sphereIntersect(vec3 origin, vec3 dir, float radius, out float tmin, out float tmax)
{
	float a = dot(dir, dir);
	float b = dot(dir, origin);
	float c = dot(origin, origin) - radius * radius;
	float h = b * b - c;

	if (h < 0)
		return false;

	float s = sqrt(h);
	tmin = -b - s;
	tmax = -b + s;
	return true;
}

bool lightRay(vec3 origin, vec3 dir, out float odr, out float odm)
{
	float tmin, tmax;
	sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax);

	float segmentLength = tmax / numLightSamples;
	float t = 0;

	odr = 0;
	odm = 0;
	for (int i = 0; i < numLightSamples; i++)
	{
		vec3 samplePosition = origin + (t + segmentLength * 0.5) * dir;
		float height = length(samplePosition) - planetRadius;
		if (height < 0)
			return false;

		float hr = exp(-height / rayleighHeightScale) * segmentLength;
		float hm = exp(-height / mieHeightScale) * segmentLength;
		odr += hr;
		odm += hm;

		t += segmentLength;
	}

	return true;
}

vec3 atmosphere(vec3 dir, vec3 lightDir)
{
	vec3 origin = vec3(0, planetRadius + 1, 0);

	float tmin, tmax;
	if (!sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax) || tmax < 0)
		return vec3(0); // space color
	if (tmin < 0)
		tmin = 0;
	
	float segmentLength = (tmax - tmin) / numSamples;
	float t = tmin;

	vec3 rayleigh = vec3(0), mie = vec3(0);
	float odr = 0, odm = 0; // cumulative optical depth

	vec3 toLight = -lightDir;
	float m = dot(dir, toLight);
	float phaseR = 3 / (16 * pi) * (1 + m * m);
	float phaseM = 3 / (8 * pi) * ((1 - mieAnisotropy * mieAnisotropy) * (1 + m * m)) /
		((2 + mieAnisotropy * mieAnisotropy) * pow(1 + mieAnisotropy * mieAnisotropy - 2 * mieAnisotropy * m, 1.5));

	for (int i = 0; i < numSamples; i++)
	{
		vec3 samplePosition = origin + (t + segmentLength * 0.5) * dir;
		float height = length(samplePosition) - planetRadius;

		// optical depth
		float hr = exp(-height / rayleighHeightScale) * segmentLength;
		float hm = exp(-height / mieHeightScale) * segmentLength;
		odr += hr;
		odm += hm;

		float odri = 0, odmi = 0;
		if (lightRay(samplePosition, toLight, odri, odmi))
		{
			vec3 tau = rayleighScatter * (odr + odri) + mieScatter * 1.1 * (odm + odmi);
			vec3 attenuation = exp(-tau);
			rayleigh += attenuation * hr;
			mie += attenuation * hm;
		}

		t += segmentLength;
	}

	float sunIntensity = 20;
	vec3 scattering = (rayleigh * rayleighScatter * phaseR + mie * mieScatter * phaseM) * sunIntensity;

	return scattering;
}

vec3 reconstructView(vec2 uv, mat4 projectionInv)
{
	vec2 ndc = vec2(uv.x * 2 - 1, uv.y * -2 + 1);

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

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth != 0)
		discard;

	vec3 view = reconstructView(v_texcoord, projectionInv); // view space direction
	view = mat3(viewInv) * view;

	vec3 color = atmosphere(view, lightDirection);

	out_color = vec4(color, 1);
}
