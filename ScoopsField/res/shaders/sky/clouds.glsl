




/*
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
}

float coveragenoise(vec3 pos)
{
	return texture(s_cloudCoverage, pos.xz / vec2(128, 128)).r;
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
	//p += vec3(5e2 * t, 0, 6e2 * t);

	p *= cloudNoiseScale;
	p *= 0.5;

    float cld = 0;

	cld += noise(p * 2);

	//bool hasCloud = cld + (cloudCoverage - 0.5) * 0.5 > 0.53;
	//if (!hasCloud)
	//	return 0;

	cld *= 0.8;
	p = p * 3.02;
	p.xz += t * 0.75;
	//cld = remap(cld / 0.99, 0.2 * noise(p), 1, 0, 1);
	cld += 0.2 * noise(p);

	//bool hasCloud2 = cld + (cloudCoverage - 0.5) * 0.5 > 0.53;
	//if (!hasCloud2)
	//	return 0;
	
	p = p * 9.01;
	p.xz -= t * 4; // detail noise moves at a different speed from cloud shapes
	cld = remap(cld / 0.97, 0.06875 * noise(p), 1, 0, 1);

	cld += (cloudCoverage - 0.5) * 0.5;

	cld = smoothstep(0.6, 1, cld);
	cld *= cld * cloudDensity;

	float heightPercentage = clamp((height - minCloudHeight) / (maxCloudHeight - minCloudHeight), 0, 1);
	float heightGradient = exp(-heightPercentage * 10); //cos(pi * 0.5 * heightPercentage);

	return cld * heightGradient;
}

float clouds2(vec3 p, float height, float t)
{
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




	/*
	float lowFrequency = texture(s_cloudLowFrequency, p.xzy / vec3(512, 512, 64) * 5).r;
	//float highFrequency = texture(s_cloudHighFrequency, p.xzy / vec3(32, 32, 32) * 50).r;
	float baseDensity = lowFrequency; //remap(lowFrequency, highFrequency, 1, 0, 1);

	float coverage = texture(s_cloudCoverage, p.xz / vec2(128, 128) * 0.25).r;
	float heightGradient = 1.0; //sin(pi * (height - minCloudHeight) / (maxCloudHeight - minCloudHeight));

	float cld = remap(baseDensity * heightGradient, coverage, 1, 0, 1);

	return cld * cloudDensity;
	/

	//float cld = fnoise(p * cloudNoiseScale, t); // + (cloudCoverage - 0.5) * 0.5;
	//cld = smoothstep(0.44, 0.64, cld);
	//cld -= 1 - cloudCoverage;
	//cld *= cld * cloudDensity;
	//return cld;
}
*/







#define cloudAnisotropy 0.9
#define minCloudHeight 2e3
#define maxCloudHeight 6e3
#define cloudCoverage 0.5
#define cloudDensity 0.0015
#define cloudNoiseScale 2e-4
#define cloudScatter 0.5
#define cloudAbsorption 0.05


bool cloudLayerIntersect(vec3 origin, vec3 dir, out float tmin, out float tmax)
{
	float x, y;
	if (!sphereIntersect(origin, dir, planetRadius + maxCloudHeight, x, y))
		return false;

	tmin = x;
	tmax = y;

	if (!sphereIntersect(origin, dir, planetRadius + minCloudHeight, x, y))
		return true;

	if (x > 0)
		tmax = min(tmax, x);
	else
		tmin = max(tmin, y);

	if (sphereIntersect(origin, dir, planetRadius, x, y) && x < tmin)
		return false;

	return true;
}

float HG(float m, float g)
{
	return 1 / (4 * pi) * (1 - g * g) / pow(1 + g * g - 2 * g * m, 1.5);
}

float cloudPhase(float m)
{
	float g = 0.0;
	return HG(m, g);

	/*
	float g = 0.9;
	float k = 0.6;
	return mix(HG(m, -g), HG(m, g), k);
	*/
}

float noise(vec3 pos)
{
	return texture(s_cloudLowFrequency, pos.xzy / vec3(256, 256, 64)).r;
}

float getCloudDensity(vec3 p, float height)
{
	float t = gameTime;

	p += vec3(5e1 * t, 0, 6e1 * t);

	p *= cloudNoiseScale;
	p *= 0.5;

    float cld = 0;

	//cld += 0.5000 * noise(p); p = p * 3.02; // p.y -= t*.1*10; //t*.05 speed cloud changes
	//cld += 0.2500 * noise(p); p = p * 3.03; // p.y += t*.06*10;
	//cld += 0.1250 * noise(p); p = p * 3.01;  p.xz += t * 2; // detail noise moves at a different speed from cloud shapes
	//cld += 0.0625 * noise(p); p =  p * 3.03;  p.xz -= t * 4; // detail noise moves at a different speed from cloud shapes
	//cld += 0.03125 * noise(p); p =  p * 3.02;
	//cld += 0.015625 * noise(p);

	cld += 0.750000 * noise(p * 3); p = p * 9.02; p.xz += t * 0.1;
	cld += 0.187500 * noise(p); p = p * 9.01; p.xz -= t * 0.4; // detail noise moves at a different speed from cloud shapes
	cld = remap(cld / 0.95, 0.06875 * noise(p), 1, 0, 1); //; p = p * 9.03;

	cld += (cloudCoverage - 0.5) * 0.5;
	cld = remap(cld, 0.5, 1, 0, 1);

	//cld = smoothstep(0.48, 0.6, cld);
	//cld *= cld * cloudDensity;

	float heightPercentage = (height - minCloudHeight) / (maxCloudHeight - minCloudHeight);
	//float heightGradient = exp(-heightPercentage * 3);
	float heightGradient = sin(pi * heightPercentage);
	cld *= heightGradient;

	cld = clamp(cld, 0, 1);

	return cld;
}

float lightRay(vec3 origin, vec3 dir, SkySettings sky)
{
	float tmin, tmax;
	sphereIntersect(origin, dir, planetRadius + maxCloudHeight, tmin, tmax);

	int numSamples = 8;
	float segmentLength = tmax / numSamples;

	float totalDensity = 0;

	for (int i = 0; i < numSamples; i++)
	{
		float t = (i + 0.5) * segmentLength;
		vec3 pos = origin + t * dir;

		float height = length(pos) - planetRadius;
		float density = getCloudDensity(pos, height);

		totalDensity += density * segmentLength;
	}

	return totalDensity;
}

vec4 clouds(vec3 origin, vec3 dir, vec3 lightDir, in SkySettings sky)
{
	origin.y += planetRadius;

	float tmin, tmax;
	if (!cloudLayerIntersect(origin, dir, tmin, tmax))
		return vec4(0);

	vec3 toLight = -lightDir;

	int numSamples = 64;
	float segmentLength = (tmax - tmin) / numSamples;

	float totalDensity = 0;
	vec3 energy = vec3(0);

	float viewTransmittance = 1;

	for (int i = 0; i < numSamples; i++)
	{
		float xi = sky.noise;
		float t = tmin + (i + 0.5 + xi) * segmentLength;

		vec3 pos = origin + t * dir;

		float height = length(pos) - planetRadius;
		float density = getCloudDensity(pos, height);

		float densityToLight = lightRay(pos, toLight, sky);
		float beer = exp(-densityToLight * cloudAbsorption);
		float powder = remap(1 - exp(-density * 2), 0, 1, 0.5, 1);
		float lighting = 0.03 + beer * powder;

		vec3 atmosphereTransmittance = sampleTransmittanceLUT(height, toLight, normalize(pos));

		energy += density * segmentLength * viewTransmittance * lighting * atmosphereTransmittance;

		viewTransmittance *= exp(-density * segmentLength * cloudScatter);

		totalDensity += density * segmentLength;
	}

	float ambient = 0.0;
	float phaseC = cloudPhase(dot(dir, toLight));
	energy = ambient + energy * phaseC;

	float sunIntensity = 25;
	vec3 color = energy * sunIntensity;

	return vec4(color, 1 - viewTransmittance);
}
