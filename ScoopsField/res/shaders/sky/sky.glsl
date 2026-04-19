


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
#define haze 0.1
#define sunConcentration 0.999


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

void getDensities(vec3 position, float height, float t, out float hr, out float hm, out float ho)
{
	hr = exp(-height / rayleighHeightScale);
	hm = exp(-height / mieHeightScale) + haze;
	ho = max(1 - abs(height - 10e3) / 10e3, 0);

	/*
	if (height > minCloudHeight && height < maxCloudHeight)
	{
		hc += clouds(position, height, t);
	}
	*/
}

void getDensitiesLod(vec3 position, float height, float t, out float hr, out float hm, out float ho)
{
	hr = exp(-height / rayleighHeightScale);
	hm = exp(-height / mieHeightScale) + haze;
	ho = max(1 - abs(height - 10e3) / 10e3, 0);

	/*
	if (height > minCloudHeight && height < maxCloudHeight)
	{
		hc += cloudsLod(position, height, t);
	}
	*/
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

float phase(float cosTheta, float g)
{
	//return 3 / (8 * pi) * ((1 - g * g) * (1 + cosTheta * cosTheta)) / ((2 + g * g) * pow(1 + g * g - 2 * g * cosTheta, 1.5));
	return 1 / (4 * pi) * (1 - g * g) / pow(1 + g * g - 2 * g * cosTheta, 1.5);
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

	vec3 rayleigh = vec3(0), mie = vec3(0), cloud = vec3(0);
	float odr = 0, odm = 0, odo = 0; // cumulative optical depth

	vec3 toLight = -lightDir;
	float m = dot(dir, toLight);
	float phaseR = 3 / (16 * pi) * (1 + m * m);
	float phaseM = phase(m, mieAnisotropy);
	//float phaseC = phase(m, cloudAnisotropy);

	float ms = m > 0.9998 ? 0.9999 : m;
	float phaseS = phase(ms, sunConcentration);
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

			/*
			float Tc = exp(-0.6 * (odc + odci));
			float k = 0.01;
			vec3 multiScatter = k * (1 - Tc) * vec3(0.2, 0.25, 0.3);
			float inScattering = 1 - exp(-hc * 0.002);
			*/

			rayleigh += attenuation * hr;
			mie += attenuation * hm;
			//cloud += attenuation * hc; // + inScattering * 20.0; // + inScattering * 0.0;
		}
	}

	float sunIntensity = 25;
	vec3 scattering = (rayleigh * rayleighScatter * phaseR + mie * mieScatter * phaseM + mie * mieScatter * phaseS) * sunIntensity;

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

		float hr, hm, ho, hc;
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
