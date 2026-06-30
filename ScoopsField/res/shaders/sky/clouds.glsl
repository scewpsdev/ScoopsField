




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
#define maxCloudHeight 10e3
//#define cloudCoverage 0.25
//#define cloudScatter 0.0625


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
	//return numericalMieFit(m) * 0.2;
	//float g = 0.7;
	//return HG(m, g);
	return max(HG(m, 0.5), HG(m, 0.9));

	//float g = 0.8;
	//float k = 0.95;
	//return mix(HG(m, -g), HG(m, g), k);
}

/*
float noise(vec3 pos)
{
	return texture(s_cloudLowFrequency, pos.xzy / vec3(256, 256, 64)).r;
}
*/


// 
// stratus:
// max height 2e3
// erosion 0.2
// anvil amount 0
// low frequency coverage
// structure erosion 0.01
// density multiplier 1
// 
// cumulus:
// max height 4e3
// erosion 0.2
// anvil amount 0
// high frequency coverage
// structure erosion 0.03
// density multiplier 1
// 
// cumulonimbus:
// max height 10e3
// erosion 0.5
// anvil amount 1
// low frequency coverage
// structure erosion 0.03
// density multiplier 5
// 

float interpolate3(float a, float b, float c, float t)
{
	t = smoothstep(0, 1, t);

	float ta = max(1 - t, 0);
	float tb = 1 - abs(t - 1);
	float tc = max(t - 1, 0);
	return ta * a + tb * b + tc * c;
}

void getCloudParams(float cloudType, out float minHeight, out float maxHeight, out float densityMultiplier, out float structureErosionMultiplier, out float detailStrengthMultiplier, out float heightGradientType)
{
	float f = fract(cloudType);

	minHeight = interpolate3(1.5e3, 2e3, 1.5e3, cloudType);
	maxHeight = interpolate3(2e3, 4e3, 10e3, cloudType);
	densityMultiplier = interpolate3(1, 1, 3, cloudType);
	structureErosionMultiplier = interpolate3(0.01, 0.01, 0.02, cloudType);
	detailStrengthMultiplier = interpolate3(0.01, 0.07, 0.01, cloudType);

	return;

	if (cloudType < 1) // stratus
	{
		minHeight = mix(1.5e3, 2e3, f);
		maxHeight = mix(2e3, 4e3, f);
		densityMultiplier = mix(0.01, 0.1, f);
		structureErosionMultiplier = 0.1;
		detailStrengthMultiplier = 0.05;
	}
	else if (cloudType < 2) // cumulus
	{
		minHeight = mix(2e3, 2.2e3, f); //1.5e3;
		maxHeight = mix(4e3, 5e3, f);
		densityMultiplier = mix(0.1, 1, f);
		structureErosionMultiplier = mix(0.1, 0.3, f);
		detailStrengthMultiplier = mix(0.05, 0.1, f);
	}
	else // cumulonimbus
	{
		minHeight = mix(2.2e3, 1.5e3, f);
		maxHeight = mix(5e3, 8e3, f);
		structureErosionMultiplier = mix(0.3, 0.7, f);
		detailStrengthMultiplier = mix(0.1, 0.04, f);
	}
}

float getCloud(float cloudType, float height, float minHeight, float maxHeight, float heightFraction, vec4 baseNoise)
{
	float highFrequencyCoverage = baseNoise.g;
	float lowFrequencyCoverage = baseNoise.b;
	float coverage = interpolate3(baseNoise.b, baseNoise.g, baseNoise.b, cloudType);

	coverage += mix(-0.45, -0.15, cloudCoverage);
	coverage = max(coverage, 0);

	float cloud = coverage; //baseNoise.b; //mix(baseNoise.g, baseNoise.b, min(abs(cloudType - 1.5), 1)); // coverage

	float topErosion = heightFraction * heightFraction * baseNoise.a;

	float totalHeightFraction = remap(height, minCloudHeight, maxCloudHeight);

	float anvilAmount = interpolate3(0, 0, 1, cloudType); //linearstep(2, 3, cloudType);
	float anvilErosion = (1 - smax(totalHeightFraction * totalHeightFraction, 0.8 * 0.8, 0.02)) * (1 - baseNoise.a);
	anvilErosion += 0.1;
	anvilErosion = mix(1, anvilErosion, anvilAmount);
	topErosion = min(topErosion, anvilErosion);

	float bottomErosion = pow(1 - heightFraction, 16);
	float erosion = bottomErosion + topErosion;

	float erosionStrength = interpolate3(0.2, 0.2, 0.5, cloudType);
	float gradient = 1; //linearstep(0, 0.2, heightFraction) - linearstep(0.8, 1.2, heightFraction);
	cloud = remap(cloud - erosion * erosionStrength, 1 - gradient, 1);
	cloud = max(cloud, 0);

	return cloud;

	float f = cloudType - floor(cloudType);

	if (cloudType < 1)
	{
		float topErosion = heightFraction * heightFraction * baseNoise.a;
		float bottomErosion = pow(1 - heightFraction, 16);
		float erosion = bottomErosion + topErosion;

		float gradient = linearstep(0, 0.2, heightFraction) - linearstep(0.7, 1.0, heightFraction);

		return remap(cloud - erosion, 1 - gradient, 1);
	}
	else if (cloudType < 2)
	{
		float topErosion = heightFraction * heightFraction * baseNoise.a;
		float bottomErosion = pow(1 - heightFraction, 16);
		float erosion = bottomErosion + topErosion;

		float gradient = linearstep(0, 0.2, heightFraction) - linearstep(0.8, 1.2, heightFraction);

		return remap(cloud - erosion, 1 - gradient, 1);
	}
	else
	{
		float topErosion = heightFraction * heightFraction * baseNoise.a * 0.5;

		float bottomErosion = pow(1 - heightFraction, 16) * 0.5;
		float erosion = bottomErosion + topErosion;

		float lowerBound = mix(0.2, 0.05, f);
		float gradient = linearstep(0, lowerBound, heightFraction) - linearstep(0.8, 1.2, heightFraction);

		cloud = remap(cloud - erosion, 1 - gradient, 1);

		float anvilWeight = linearstep(0.9, 1, f * f);
		float totalHeightFraction = remap(height, minHeight, maxCloudHeight);
		float anvil = linearstep(0.7, 1, totalHeightFraction * totalHeightFraction) * f * 0.1 * anvilWeight;
		float anvilErosion = (1 - totalHeightFraction * totalHeightFraction) * baseNoise.a * 2;
		anvil -= anvilErosion;
		//anvil = max(anvil, 0);

		cloud = max(cloud, anvil);
		cloud += anvilWeight * 0.1;

		return cloud;
	}
}

float getCloudDensity2(vec3 p, float height, int lod)
{
	vec3 windOffset = vec3(5e2, 0, 6e2) * gameTime * windSpeed;
	float baseNoiseScale = 0.000005 * 1.51;
	float structureNoiseScale = 0.0001 * 1.51;
	float detailNoiseScale = 0.0005 * 1.51;

	vec3 leanOffset = 0.8 * vec3(5e3, 0, 6e3) * pow(remap(height, minCloudHeight, maxCloudHeight), 2);

	vec3 baseCoord = (p + windOffset + leanOffset) * baseNoiseScale + 0.5;
	vec3 structureCoord = (p + windOffset * 1.25) * structureNoiseScale + 0.5;
	vec3 detailCoord = (p + windOffset * 1.5) * detailNoiseScale + 0.5;

	vec4 baseNoise = texture(s_weatherMap, baseCoord.xz);

	float cloudType = clamp(baseNoise.r * 2, 0, 1.99999);

	float minHeight, maxHeight, densityMultiplier, structureErosionMultiplier, detailStrengthMultiplier, heightGradientType;
	getCloudParams(cloudType, minHeight, maxHeight, densityMultiplier, structureErosionMultiplier, detailStrengthMultiplier, heightGradientType);

	if (height < minHeight || height > maxHeight && cloudType < 2)
		return 0;

	float heightFraction = remap(height, minHeight, maxHeight);

	float cloud = getCloud(cloudType, height, minHeight, maxHeight, heightFraction, baseNoise);
	
	//float cloud = remap(baseNoise.r - erosion, 1 - cloudGradient /*baseNoise.g*/, 1);
	//cloud = max(cloud, 0);
	//cloud *= heightMask;

	if (cloud > 0)
	{
		vec4 structureNoise = texture(s_cloudNoise, structureCoord);
		float structureErosion = 1 - structureNoise.r; //(structureNoise.r + 0.5 * structureNoise.g + 0.25 * structureNoise.b + 0.125 * structureNoise.a) / 1.875;
		cloud = remap(cloud, structureErosion * structureErosionMultiplier, 1);
		cloud = max(cloud, 0);

		float structureDetail = 1 - 0.625 * structureNoise.g + 0.25 * structureNoise.b + 0.125 * structureNoise.a;
		//cloud = remap(cloud, (structureDetail - 0.9) * 0.05, 1);
		cloud = max(cloud, 0);

		float detailStrength = smoothstep(1, 0.5, cloud) * detailStrengthMultiplier;
		if (cloud > 0 && detailStrength > 0)
		{
			vec3 detailNoise = texture(s_cloudNoiseDetail, detailCoord).rgb;
			float detailErosion = detailNoise.r * 0.625 + detailNoise.g * 0.25 + detailNoise.b * 0.125;
			detailErosion = heightFraction < 0.5 || height > 0.5 * maxCloudHeight ? 1 - detailErosion : detailErosion;
			cloud = remap(cloud, detailErosion * detailStrength, 1);
			cloud = max(cloud, 0);

			//cloud -= detailErosion * detailStrength;
			//detailErosion = mix(detailErosion, 1 - detailErosion, 0.35);
			//cloud = remap(cloud, detailErosion * 0.2, 1, 0, 1);
		}
	}

	//cloud += mix(0.5, 1, 0.3) - 1;

	//cloud = smoothstep(0, 0.1, cloud);
	cloud *= linearstep(0, 0.25, heightFraction);

	cloud *= cloudDensity * densityMultiplier;

	return cloud;
}

float getCloudDensity(vec3 p, float height, int lod)
{
	vec3 windOffset = vec3(5e2, 0, 6e2) * gameTime * windSpeed;
	float baseNoiseScale = 0.00005;
	float detailNoiseScale = 0.00066;

	vec3 baseCoord = (p + windOffset) * baseNoiseScale;
	vec3 detailCoord = (p + windOffset * 1.5) * detailNoiseScale;

	vec4 baseNoise = texture(s_weatherMap, baseCoord.xz);

	float pw = baseNoise.r;
	vec3 worley = baseNoise.gba;

	float heightFraction = remap(height, minCloudHeight, maxCloudHeight, 0, 1);
	float baseErosion = mix(worley.r, worley.g, heightFraction);
	float cloud = remap(pw, baseErosion * 0.5, 1, 0, 1);

	float heightMask = smoothstep(0, 0.1, heightFraction) * smoothstep(1, 0.7, heightFraction);
	cloud *= heightMask;

	float threshold = 0.2; //1 - cloudCoverage;
	cloud = remap(cloud, threshold, 1, 0, 1);
	cloud = max(cloud, 0);

	if (cloud > 0)
	{
		vec3 detailNoise = texture(s_cloudNoiseDetail, detailCoord).rgb;
		float detailErosion = detailNoise.r * 0.625 + detailNoise.g * 0.25 + detailNoise.b * 0.125;
		detailErosion = mix(detailErosion, 1 - detailErosion, 0.35);
		cloud = remap(cloud, detailErosion * 0.2, 1, 0, 1);
		cloud = max(cloud, 0);
	}

	cloud *= 10;

	return cloud;

	/*
	float t = gameTime;

	p += vec3(5e2 * t, 0, 6e2 * t) * windSpeed;

	p *= 2e-4 * 0.25;

	float heightGradient = remap(height, minCloudHeight, maxCloudHeight, 0, 1);

	vec4 perlinWorley = texture(s_cloudNoise, p * 0.5);

	float threshold = 1 - cloudCoverage;

	float cloud = perlinWorley.x;
	if (cloud < threshold)
		return 0;

	if (false)
	//if (lod <= 1)
	{
		vec3 worley = perlinWorley.yzw; //texture(s_cloudNoise, p * 0.25 + t * 0.005).yzw;
		float wfbm = worley.x * 0.625 + worley.y * 0.25 + worley.z * 0.125;
		cloud = remap(cloud, 0, 1, wfbm - 1, 1);

		if (cloud < threshold)
			return 0;

		if (lod == 0)
		{
			vec3 detail = texture(s_cloudNoiseDetail, p * 8 + t * 0.3 * windSpeed).xyz;
			float dfbm = detail.x * 0.625 + detail.y * 0.25 + detail.z * 0.125;
			float billowRemap = remap(cloud, 0, 1, dfbm - 1, 1);
			float whispyRemap = remap(cloud, dfbm - 0.5, 1, 0, 1);
			cloud = mix(whispyRemap, billowRemap, smoothstep(0.4, 0.6, heightGradient));
		}
	}

	//cloud -= max(abs(heightGradient * 2 - 1) - 0.5, 0) * 0.5;

	cloud = smoothstep(threshold, 1, cloud);
    //cloud = remap(cloud, threshold, 1, 0, 1);
	
	//heightGradient *= heightGradient;
	//heightGradient *= sin(heightGradient * pi);
	//heightGradient *= sin(heightGradient * pi);
	cloud *= 1 - max(abs(heightGradient * 2 - 1) - 0.5, 0) * 2;
	//cloud *= 1 - max(abs(heightGradient * 2 - 1) - 0.5, 0) * 2;
	//cloud *= sin(heightGradient * pi);
	//cloud *= heightGradient * heightGradient;
	//cloud *= heightGradient * heightGradient;

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

float lightRay(vec3 origin, vec3 dir, float mu, float noise, int lod)
{
	float tmin, tmax;
	if (!cloudLayerIntersect(origin, dir, tmin, tmax))
		return 0;
	//sphereIntersect(origin, dir, planetRadius + maxCloudHeight, tmin, tmax);

	int numSamples = 32;
	float ldt = 1.0 / numSamples;

	float totalDensity = 0;

	for (int i = 0; i < numSamples; i++)
	{
		float xi = noise;
		float u0 = (i + xi) * ldt;
		float u1 = (i + 1 + xi) * ldt;
		float u = (i + 0.5 + xi) * ldt;

		float t0 = tmax * u0 * u0;
		float t1 = tmax * u1 * u1;
		float t = tmax * u * u;

		float dt = t1 - t0;

		vec3 pos = origin + t * dir;

		float height = length(pos) - planetRadius;
		float density = getCloudDensity2(pos, height, lod);

		totalDensity += density * dt;
	}

	return totalDensity;
}

vec4 clouds(vec3 origin, vec3 dir, vec3 lightDir, float noise, int lod, int numSamples)
{
	origin.y += planetRadius;

	float tmin, tmax;
	if (!cloudLayerIntersect(origin, dir, tmin, tmax))
		return vec4(0, 0, 0, 1);

	float maxDistance = 200e3;
	float lodDistance = 50e3;
	float lodDistance2 = 90e3;
	//tmin = min(tmin, maxDistance);
	//tmax = min(tmax, maxDistance);

	vec3 toLight = -lightDir;

	float mu = dot(dir, toLight);
	float phaseC = cloudPhase(mu);

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

		lod = max(lod, clamp(int(floor((t - lodDistance) / (lodDistance2 - lodDistance))), -1, 1) + 1);

		vec3 pos = origin + t * dir;

		float height = length(pos) - planetRadius;
		float density = getCloudDensity2(pos, height, lod);
		density *= segmentLength;

		vec3 localUp = normalize(pos);

		if (density > 0)
		{
			float densityToLight = lightRay(pos, toLight, mu, noise, lod);
			float beer = exp(-densityToLight);

			float fakeScatter = mix(0.008, 1, smoothstep(0.96, 0, mu));
			beer += 0.5 * fakeScatter * exp(-0.1 * densityToLight);
			beer += 0.4 * fakeScatter * exp(-0.02 * densityToLight);
			//beer *= mix(0.05 + 1.5 * pow(min(1, density / segmentLength * 8.5), 0.3 + 5.5 * clamp(remap(height, minCloudHeight, maxCloudHeight, 0, 1), 0, 1)), 1, clamp(densityToLight * 0.4, 0, 1));

			vec3 sunlight = sampleTransmittanceLUT(height, toLight, localUp);
			vec3 multiScatter = sampleMultiScatter(height, toLight, localUp);

			float ambient = 0.006;
			float powder = 1 - exp(-density);
			float shadow = beer * powder;

			vec3 lighting = (sunlight * phaseC) * shadow + ambient * sunlight + multiScatter / (4 * pi);

			//vec3 lighting = sunlight * (ambient + beer * powder) * phaseC;

			energy += transmittance * lighting;

			totalDensity += density;
			transmittance = exp(-totalDensity * cloudDensity);

			dist = t;

			if (transmittance < 0.01)
				break;
		}
		else
		{
			vec3 sunlight = sampleTransmittanceLUT(height, toLight, localUp);

			float ambient = 0.006;
			//energy += transmittance * ambient * sunlight * phaseC;
		}
	}

	float sunIntensity = 25;
	vec3 color = energy * sunIntensity;

	vec4 aerial = calculateAerial(origin - vec3(0, planetRadius, 0), dir, dist, lightDir);
	float inscatterMultiplier = mix(1, 0.1, cloudCoverage);
	inscatterMultiplier = mix(inscatterMultiplier, 1, clamp(remap(dist * dir.y, minCloudHeight, maxCloudHeight), 0, 1));
	color = color * aerial.a + aerial.rgb * inscatterMultiplier;
	//transmittance *= aerial.a;

	return vec4(color, transmittance);
}

vec4 clouds(vec3 origin, vec3 dir, vec3 lightDir, float noise)
{
	return clouds(origin, dir, lightDir, noise, 0, 128);
}
