




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







#define minCloudHeight 1.5e3
#define maxCloudHeight 4e3
#define cloudCoverage 0.25
#define cloudScatter 0.0625


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

	if (sphereIntersect(origin, dir, planetRadius, x, y) && x > 0 && x < tmin)
		return false;

	return true;
}

float HG(float m, float g)
{
	return 1 / (4 * pi) * (1 - g * g) / pow(1 + g * g - 2 * g * m, 1.5);
}

// From https://www.shadertoy.com/view/4sjBDG
float numericalMieFit(float costh)
{
    // This function was optimized to minimize (delta*delta)/reference in order to capture
    // the low intensity behavior.
    float bestParams[10];
    bestParams[0]=9.805233e-06;
    bestParams[1]=-6.500000e+01;
    bestParams[2]=-5.500000e+01;
    bestParams[3]=8.194068e-01;
    bestParams[4]=1.388198e-01;
    bestParams[5]=-8.370334e+01;
    bestParams[6]=7.810083e+00;
    bestParams[7]=2.054747e-03;
    bestParams[8]=2.600563e-02;
    bestParams[9]=-4.552125e-12;
    
    float p1 = costh + bestParams[3];
    vec4 expValues = exp(vec4(bestParams[1] *costh+bestParams[2], bestParams[5] *p1*p1, bestParams[6] *costh, bestParams[9] *costh));
    vec4 expValWeight= vec4(bestParams[0], bestParams[4], bestParams[7], bestParams[8]);
    return dot(expValues, expValWeight);
}

float cloudPhase(float m)
{
	//return numericalMieFit(m);
	float g = 0.5;
	return HG(m, g);

	//float g = 0.5;
	//float k = 0.6;
	//return mix(HG(m, -g), HG(m, g), k);
}

float noise(vec3 pos)
{
	return texture(s_cloudLowFrequency, pos.xzy / vec3(256, 256, 64)).r;
}

float getCloudDensity(vec3 p, float height, int lod)
{
	float t = gameTime;

	p += vec3(5e1 * t, 0, 6e1 * t);

	p *= 2e-4 * 0.5;

	float heightGradient = remap(height, minCloudHeight, maxCloudHeight, 0, 1);

	float perlinWorley = texture(s_cloudNoise, p * 0.25).x;

	float threshold = 1 - cloudCoverage;

	float cloud = perlinWorley;
	if (cloud < threshold)
		return 0;

	if (lod <= 1)
	{
		vec3 worley = texture(s_cloudNoise, p * 0.5 + t * 0.005).yzw;
		float wfbm = worley.x * 0.625 + worley.y * 0.125 + worley.z * 0.25;
		cloud = remap(cloud, 0, 1, wfbm - 1, 1);

		if (cloud < threshold)
			return 0;

		if (lod == 0)
		{
			vec3 detail = texture(s_cloudNoiseDetail, p * 4 + t * 0.01).xyz;
			float dfbm = detail.x * 0.625 + detail.y * 0.25 + detail.z * 0.125;
			float billowRemap = remap(cloud, 0, 1, dfbm - 1, 1);
			float whispyRemap = remap(cloud, dfbm - 0.5, 1, 0, 1);
			cloud = mix(whispyRemap, billowRemap, smoothstep(0.5, 0.7, heightGradient));
		}
	}

	cloud = smoothstep(threshold, 1, cloud);
    //cloud = remap(cloud, threshold, 1, 0, 1);
	
	heightGradient *= heightGradient;
	heightGradient *= sin(heightGradient * pi);
	heightGradient *= sin(heightGradient * pi);
	cloud *= sin(heightGradient * pi);
	cloud *= heightGradient;

	cloud = clamp(cloud, 0, 1);

	return cloud;

	/*
    float cld = 0;

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
	*/
}

float getCloudDensityLight(vec3 p, float height, int lod)
{
	float t = gameTime;

	p += vec3(5e1 * t, 0, 6e1 * t);

	p *= 2e-4 * 0.5;

	float heightGradient = remap(height, minCloudHeight, maxCloudHeight, 0, 1);

	float perlinWorley = texture(s_cloudNoise, p * 0.25).x;

	float threshold = 1 - cloudCoverage;

	float cloud = perlinWorley;
	if (cloud < threshold)
		return 0;

	if (lod <= 1)
	{
		vec3 worley = texture(s_cloudNoise, p * 0.5 + t * 0.005).yzw;
		float wfbm = worley.x * 0.625 + worley.y * 0.125 + worley.z * 0.25;
		cloud = remap(cloud, 0, 1, wfbm - 1, 1);

		if (cloud < threshold)
			return 0;

		if (lod == 0)
		{
			vec3 detail = texture(s_cloudNoiseDetail, p * 4 + t * 0.01).xyz;
			float dfbm = detail.x * 0.625 + detail.y * 0.25 + detail.z * 0.125;
			float billowRemap = remap(cloud, 0, 1, dfbm - 1, 1);
			float whispyRemap = remap(cloud, dfbm - 0.5, 1, 0, 1);
			cloud = mix(whispyRemap, billowRemap, smoothstep(0.5, 0.7, heightGradient));
		}
	}

	cloud = smoothstep(threshold, 1, cloud);
    //cloud = remap(cloud, threshold, 1, 0, 1);
	
	heightGradient *= heightGradient;
	cloud *= sin(heightGradient * pi);
	cloud *= heightGradient;

	cloud = clamp(cloud, 0, 1);

	return cloud;
}

float lightRay(vec3 origin, vec3 dir, float mu, int lod)
{
	float tmin, tmax;
	cloudLayerIntersect(origin, dir, tmin, tmax);
	//sphereIntersect(origin, dir, planetRadius + maxCloudHeight, tmin, tmax);

	int numSamples = 6;
	float ldt = 1.0 / numSamples;

	float totalDensity = 0;

	for (int i = 0; i < numSamples; i++)
	{
		float u0 = (i) * ldt;
		float u1 = (i + 1) * ldt;
		float u = (i + 0.5) * ldt;

		float t0 = tmax * u0 * u0;
		float t1 = tmax * u1 * u1;
		float t = tmax * u * u;

		float dt = t1 - t0;

		vec3 pos = origin + t * dir;

		float height = length(pos) - planetRadius;
		float density = getCloudDensityLight(pos, height, lod);

		totalDensity += density * dt;
	}

	return totalDensity;
}

vec4 clouds(vec3 origin, vec3 dir, vec3 lightDir, float noise, int lod)
{
	origin.y += planetRadius;

	float tmin, tmax;
	if (!cloudLayerIntersect(origin, dir, tmin, tmax))
		return vec4(0, 0, 0, 1);

	float maxDistance = 150e3;
	//tmin = min(tmin, maxDistance);
	//tmax = min(tmax, maxDistance);

	vec3 toLight = -lightDir;

	float mu = dot(dir, toLight);
	float phaseC = cloudPhase(mu);

	int numSamples = 32;
	float segmentLength = (tmax - tmin) / numSamples;
	// TODO use variable segment length and lod levels depending on current density

	float totalDensity = 0;
	vec3 energy = vec3(0);

	float transmittance = 1;
	float dist = tmin;

	for (int i = 0; i < numSamples; i++)
	{
		float xi = noise;
		float t = tmin + (i + 0.5 + xi) * segmentLength;
		if (t > maxDistance)
			break;

		vec3 pos = origin + t * dir;

		float height = length(pos) - planetRadius;
		float density = getCloudDensity(pos, height, lod);
		density *= segmentLength;

		if (density > 0)
		{
			float densityToLight = lightRay(pos, toLight, mu, lod);
			float beer = exp(-densityToLight);

			float fakeScatter = mix(0.008, 1, smoothstep(0.96, 0, mu));
			beer += 0.5 * fakeScatter * exp(-0.1 * densityToLight);
			beer += 0.4 * fakeScatter * exp(-0.02 * densityToLight);
			//beer *= mix(0.05 + 1.5 * pow(min(1, density / segmentLength * 8.5), 0.3 + 5.5 * clamp(remap(height, minCloudHeight, maxCloudHeight, 0, 1), 0, 1)), 1, clamp(densityToLight * 0.4, 0, 1));

			vec3 sunlight = sampleTransmittanceLUT(height, toLight, normalize(pos));
			vec3 multiScatter = sampleMultiScatter(height, toLight, normalize(pos));

			float ambient = 0.006;
			float powder = 1 - exp(-density);
			vec3 lighting = ambient * sunlight + beer * (sunlight + multiScatter) * powder;

			energy += transmittance * lighting;

			totalDensity += density;
			transmittance = exp(-totalDensity * cloudScatter);

			dist = t;

			if (transmittance < 0.01)
				break;
		}
		else
		{
			vec3 sunlight = sampleTransmittanceLUT(height, toLight, normalize(pos));

			float ambient = 0.006;
			energy += transmittance * ambient * sunlight;
		}
	}

	energy *= phaseC;

	float sunIntensity = 25;
	vec3 color = energy * sunIntensity;

	vec4 aerial = calculateAerial(origin - vec3(0, planetRadius, 0), dir, dist, lightDir);
	color += aerial.rgb;
	//transmittance *= aerial.a;

	return vec4(color, transmittance);
}
