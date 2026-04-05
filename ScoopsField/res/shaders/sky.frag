#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;

layout(set = 2, binding = 4) uniform sampler2D s_noise;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	mat4 projectionInv;
	mat4 viewInv;

#define lightDirection params.xyz
#define time params.w
};


#define pi 3.14159265359

#define planetRadius 6360e3
#define atmosphereRadius 6380e3
#define rayleighScatter vec3(5.8e-6f, 13.5e-6f, 33.1e-6f)
#define mieScatter 21e-6f
#define rayleighHeightScale 7994
#define mieHeightScale 1200
#define mieAnisotropy 0.45 // 0.76
#define ozoneAbsorption vec3(0.65e-6, 1.88e-6, 0.085e-6)
#define minCloudHeight 5e3
#define maxCloudHeight 8e3
#define cloudCoverage 0.5
#define cloudDensity 15.0
#define cloudNoiseScale 2e-4

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

// by iq
float noise(vec3 x)
{
	return texture(s_noise, x.xz / 512).x;

    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);

	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture( s_noise, (uv+ 0.5)/512.0, -100.0).yx;
	return mix( rg.x, rg.y, f.z );
}


float fnoise(vec3 p, float t)
{
	p *= .25;

    float f;
	f = 0.5000 * noise(p); p = p * 3.02; p.y -= t*.1; //t*.05 speed cloud changes
	f += 0.2500 * noise(p); p = p * 3.03; p.y += t*.06;
	f += 0.1250 * noise(p); p = p * 3.01;
	f += 0.0625 * noise(p); p =  p * 3.03;
	f += 0.03125 * noise(p); p =  p * 3.02;
	f += 0.015625 * noise(p);

    return f;
}

float cloud(vec3 p, float t)
{
	float cld = fnoise(p * cloudNoiseScale, t) + cloudCoverage * 0.1;
	cld = smoothstep(0.44, 0.64, cld);
	cld *= cld * cloudDensity;
	return cld;
}

void getDensities(vec3 position, float height, out float hr, out float hm, out float ho)
{
	hr = exp(-height / rayleighHeightScale);
	hm = exp(-height / mieHeightScale);
	ho = max(1 - abs(height - 10e3) / 10e3, 0);

	if (height > minCloudHeight && height < maxCloudHeight)
	{
		float cld = cloud(position + vec3(5e2 * time, 0, 6e2 * time), time);
		cld *= sin(pi * (height - minCloudHeight) / (maxCloudHeight - minCloudHeight));
		hm += cld;
	}
}

bool lightRay(vec3 origin, vec3 dir, out float odr, out float odm, out float odo)
{
	float tmin, tmax;
	sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax);

	float segmentLength = tmax / numLightSamples;
	float t = 0;

	odr = 0;
	odm = 0;
	odo = 0;
	for (int i = 0; i < numLightSamples; i++)
	{
		vec3 samplePosition = origin + (t + segmentLength * 0.5) * dir;
		float height = length(samplePosition) - planetRadius;
		if (height < 0)
			return false;

		// optical depth
		float hr, hm, ho;
		getDensities(samplePosition, height, hr, hm, ho);
		hr *= segmentLength;
		hm *= segmentLength;
		ho *= segmentLength;
		
		odr += hr;
		odm += hm;
		odo += ho;

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
	float odr = 0, odm = 0, odo = 0; // cumulative optical depth

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
		float hr, hm, ho;
		getDensities(samplePosition, height, hr, hm, ho);
		hr *= segmentLength;
		hm *= segmentLength;
		ho *= segmentLength;

		odr += hr;
		odm += hm;
		odo += ho;

		float odri = 0, odmi = 0, odoi = 0;
		if (lightRay(samplePosition, toLight, odri, odmi, odoi))
		{
			vec3 tau = rayleighScatter * (odr + odri) + mieScatter * 1.1 * (odm + odmi) + ozoneAbsorption * (odo + odoi) * 1.5;
			vec3 attenuation = exp(-tau);
			rayleigh += attenuation * hr;
			mie += attenuation * hm;
		}

		t += segmentLength;
	}

	float sunIntensity = 25;
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
