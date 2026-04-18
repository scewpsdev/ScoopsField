


struct SkySettings
{
	int numSamples;
	int numLightSamples;

	bool offsetRayStart;
	float noise;

	bool lod;

	float time;
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
#define haze 0.0
#define sunConcentration 0.9999
#define minCloudHeight 5e3
#define maxCloudHeight 8e3
#define cloudCoverage 0.5
#define cloudDensity 15
#define cloudNoiseScale 2e-4


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

/*
float coveragenoise(vec3 pos)
{
	return texture(s_cloudCoverage, pos.xz / vec2(128, 128)).r;
}
*/

float noise(vec3 pos)
{
	return texture(s_cloudLowFrequency, pos.xzy / vec3(256, 256, 64)).r;
}

float clouds(vec3 p, float height, float t)
{
	//p += vec3(5e2 * t, 0, 6e2 * t);

	p *= cloudNoiseScale;
	p *= 0.5;

    float cld = 0;

	//cld += 0.5000 * noise(p); p = p * 3.02; // p.y -= t*.1*10; //t*.05 speed cloud changes
	//cld += 0.2500 * noise(p); p = p * 3.03; // p.y += t*.06*10;
	//cld += 0.1250 * noise(p); p = p * 3.01;  p.xz += t * 2; // detail noise moves at a different speed from cloud shapes
	//cld += 0.0625 * noise(p); p =  p * 3.03;  p.xz -= t * 4; // detail noise moves at a different speed from cloud shapes
	//cld += 0.03125 * noise(p); p =  p * 3.02;
	//cld += 0.015625 * noise(p);

	cld += 0.750000 * noise(p); p = p * 9.02; p.xz += t * 0.75;
	cld += 0.187500 * noise(p); p = p * 9.01; p.xz -= t * 4; // detail noise moves at a different speed from cloud shapes
	cld = remap(cld / 0.95, 0.06875 * noise(p), 1, 0, 1); //; p = p * 9.03;

	cld += (cloudCoverage - 0.5) * 0.5;

	cld = smoothstep(0.48, 0.6, cld);
	cld *= cld * cloudDensity;

	float heightPercentage = (height - minCloudHeight) / (maxCloudHeight - minCloudHeight);
	float heightGradient = sin(pi * (0.25 + 0.75 * heightPercentage));

	return cld * heightGradient;
}

float cloudsLod(vec3 p, float height, float t)
{
	//p += vec3(5e2 * t, 0, 6e2 * t);

	p *= cloudNoiseScale;
	p *= 0.5;

    float cld = 0;

	cld += 0.8 * noise(p); p = p * 9.02; p.xz += t * 0.75;
	cld += 0.2 * noise(p); p = p * 9.01; p.xz -= t * 4; // detail noise moves at a different speed from cloud shapes

	cld += (cloudCoverage - 0.5) * 0.5;

	cld = smoothstep(0.48, 0.6, cld);
	cld *= cld * cloudDensity;

	float heightPercentage = (height - minCloudHeight) / (maxCloudHeight - minCloudHeight);
	float heightGradient = sin(pi * (0.25 + 0.75 * heightPercentage));

	return cld * heightGradient;
}

float clouds3(vec3 p, float height, float t)
{
	p += vec3(5e2 * t, 0, 6e2 * t);

	p *= cloudNoiseScale;
	p *= 0.5;

    float cld = 0;

	cld += noise(p);

	//bool hasCloud = cld + (cloudCoverage - 0.5) * 0.5 > 0.53;
	//if (!hasCloud)
	//	return 0;

	//cld *= 0.8;
	p = p * 3.02;
	p.xz += t * 0.75;
	cld = remap(cld / 0.99, 0.2 * noise(p), 1, 0, 1);
	//cld += 0.2 * noise(p);

	//bool hasCloud2 = cld + (cloudCoverage - 0.5) * 0.5 > 0.53;
	//if (!hasCloud2)
	//	return 0;
	
	p = p * 9.01;
	p.xz -= t * 4; // detail noise moves at a different speed from cloud shapes
	cld = remap(cld / 0.97, 0.06875 * noise(p), 1, 0, 1);

	cld += (cloudCoverage - 0.5) * 0.5;

	cld = smoothstep(0.53, 0.55, cld);
	cld *= cld * cloudDensity;

	float heightPercentage = (height - minCloudHeight) / (maxCloudHeight - minCloudHeight);
	float heightGradient = sin(pi * (0.25 + 0.75 * heightPercentage));

	return cld * heightGradient;
}

float clouds2(vec3 p, float height, float t)
{
	return 0;

	/*
	float lowFrequency = texture(s_cloudLowFrequency, p.xzy / vec3(256, 256, 64) * cloudNoiseScale * 20).r;
	lowFrequency = smoothstep(0.1, 0.5, lowFrequency);
	float highFrequency = texture(s_cloudHighFrequency, p.xzy / vec3(32, 32, 32) * 50).r;
	float baseDensity = remap(lowFrequency, highFrequency * 0.5, 1, 0, 1);

	float coverage = clamp(cloudCoverage - 0.5 + texture(s_cloudCoverage, p.xz / vec2(128, 128) * 2 * cloudNoiseScale).r, 0, 1);
	//coverage = step(0.5, coverage);
	float heightPercentage = (height - minCloudHeight) / (maxCloudHeight - minCloudHeight);
	float heightGradient = sin(pi * (0.25 + 0.75 * heightPercentage));

	float cld = smoothstep(0.3, 0.7, coverage) * baseDensity; //remap(0.75, 1 - coverage, 1, 0, 1);
	cld = clamp(cld, 0, 1);

	return cld * cloudDensity;
	*/




	/*
	float lowFrequency = texture(s_cloudLowFrequency, p.xzy / vec3(512, 512, 64) * 5).r;
	//float highFrequency = texture(s_cloudHighFrequency, p.xzy / vec3(32, 32, 32) * 50).r;
	float baseDensity = lowFrequency; //remap(lowFrequency, highFrequency, 1, 0, 1);

	float coverage = texture(s_cloudCoverage, p.xz / vec2(128, 128) * 0.25).r;
	float heightGradient = 1.0; //sin(pi * (height - minCloudHeight) / (maxCloudHeight - minCloudHeight));

	float cld = remap(baseDensity * heightGradient, coverage, 1, 0, 1);

	return cld * cloudDensity;
	*/

	//float cld = fnoise(p * cloudNoiseScale, t); // + (cloudCoverage - 0.5) * 0.5;
	//cld = smoothstep(0.44, 0.64, cld);
	//cld -= 1 - cloudCoverage;
	//cld *= cld * cloudDensity;
	//return cld;
}

void getDensities(vec3 position, float height, float t, out float hr, out float hm, out float ho)
{
	hr = exp(-height / rayleighHeightScale);
	hm = exp(-height / mieHeightScale) + haze;
	ho = max(1 - abs(height - 10e3) / 10e3, 0);

	if (height > minCloudHeight && height < maxCloudHeight)
	{
		hm += clouds(position, height, t);
	}
}

void getDensitiesLod(vec3 position, float height, float t, out float hr, out float hm, out float ho)
{
	hr = exp(-height / rayleighHeightScale);
	hm = exp(-height / mieHeightScale) + haze;
	ho = max(1 - abs(height - 10e3) / 10e3, 0);

	if (height > minCloudHeight && height < maxCloudHeight)
	{
		hm += cloudsLod(position, height, t);
	}
}

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
}

bool lightRay(vec3 origin, vec3 dir, in SkySettings settings, out float odr, out float odm, out float odo)
{
	float tmin, tmax;
	sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax);

	float segmentLength = tmax / settings.numLightSamples;

	odr = 0;
	odm = 0;
	odo = 0;

	for (int i = 0; i < settings.numLightSamples; i++)
	{
		float t = i * segmentLength;
		vec3 samplePosition = origin + t * dir;
		float height = length(samplePosition) - planetRadius;
		// apparently this is slow
		//if (height < 0)
		//	return false;

		// optical depth
		float hr, hm, ho;
		getDensitiesLod(samplePosition, height, settings.time, hr, hm, ho);

		hr *= segmentLength;
		hm *= segmentLength;
		ho *= segmentLength;
		
		odr += hr;
		odm += hm;
		odo += ho;
	}

	return true;
}

vec3 atmosphere(vec3 dir, vec3 lightDir, in SkySettings settings)
{
	vec3 origin = vec3(0, planetRadius + 1000, 0);

	float tmin, tmax;
	if (!sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax) || tmax < 0)
		return vec3(0); // space color
	if (tmin < 0)
		tmin = 0;

	float l = tmax - tmin;
	float ldt = 1.0 / settings.numSamples;
	float segmentLength = l * ldt;

	vec3 rayleigh = vec3(0), mie = vec3(0);
	float odr = 0, odm = 0, odo = 0; // cumulative optical depth

	vec3 toLight = -lightDir;
	float m = dot(dir, toLight);
	float phaseR = 3 / (16 * pi) * (1 + m * m);
	float phaseM = 3 / (8 * pi) * ((1 - mieAnisotropy * mieAnisotropy) * (1 + m * m)) /
		((2 + mieAnisotropy * mieAnisotropy) * pow(1 + mieAnisotropy * mieAnisotropy - 2 * mieAnisotropy * m, 1.5));

	float ms = m > 0.9998 ? 0.9999 : m;
	float phaseS = 3 / (8 * pi) * ((1 - sunConcentration * sunConcentration) * (1 + ms * ms)) /
		((2 + sunConcentration * sunConcentration) * pow(1 + sunConcentration * sunConcentration - 2 * sunConcentration * ms, 1.5));
	phaseS *= 1 - exp(-max(dir.y + 0.012, 0) * 1000);

	for (int i = 0; i < settings.numSamples; i++)
	{
		float xi = settings.offsetRayStart ? settings.noise - 0.5 : 0;

		float u0 = (i + xi) * ldt;
		float u1 = (i + 1 + xi) * ldt;
		float u = (i + 0.5 + xi) * ldt;

		float t0 = tmin + l * u0 * u0;
		float t1 = tmin + l * u1 * u1;
		float t = tmin + l * u * u;

		float dt = t1 - t0;

		vec3 samplePosition = origin + t * dir;

		// length(samplePosition) is faster than samplePosition.y ???
		float height = length(samplePosition) - planetRadius;
		if (height <= 0 && dir.y < 0)
			break;

		// optical depth
		float hr, hm, ho;
		if (settings.lod)
			getDensitiesLod(samplePosition, height, settings.time, hr, hm, ho);
		else
			getDensities(samplePosition, height, settings.time, hr, hm, ho);

		hr *= dt;
		hm *= dt;
		ho *= dt;

		odr += hr;
		odm += hm;
		odo += ho;

		float odri = 0, odmi = 0, odoi = 0;
		if (lightRay(samplePosition, toLight, settings, odri, odmi, odoi))
		{
			vec3 tau = rayleighScatter * (odr + odri) + mieScatter * (odm + odmi) + ozoneAbsorption * (odo + odoi) * 2;
			vec3 attenuation = exp(-tau);
			rayleigh += attenuation * hr;
			mie += attenuation * hm;
		}
	}

	float sunIntensity = 25;
	vec3 scattering = (rayleigh * rayleighScatter * phaseR + mie * mieScatter * phaseM + mie * mieScatter * phaseS * 5) * sunIntensity;

	return scattering;
}

vec3 attenuateSun(vec3 sunDirection, float time)
{
	vec3 origin = vec3(0, planetRadius + 1000, 0);
	vec3 dir = -sunDirection;

	int numSamples = 128;

	float odr = 0, odm = 0, odo = 0;

	float tmin, tmax;
	sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax);

	float segmentLength = tmax / numSamples;

	for (int i = 0; i < numSamples; i++)
	{
		float t = i * segmentLength;
		vec3 samplePosition = origin + t * dir;
		float height = length(samplePosition) - planetRadius;

		float hr, hm, ho;
		getDensities(samplePosition, height, time, hr, hm, ho);

		hr *= segmentLength;
		hm *= segmentLength;
		ho *= segmentLength;
		
		odr += hr;
		odm += hm;
		odo += ho;
	}

	vec3 tau = rayleighScatter * odr + mieScatter * odm + ozoneAbsorption * odo * 2;
	vec3 attenuation = exp(-tau);

	float sunIntensity = 25;
	return attenuation * sunIntensity;
}
