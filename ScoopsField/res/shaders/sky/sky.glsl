/*
struct AtmosphereData
{
	float haze;
	float cloudCoverage;
	float cloudDensity;
};
*/


#define planetRadius 6360e3
#define atmosphereRadius 6460e3
#define rayleighScatter vec3(5.8e-6f, 13.5e-6f, 33.1e-6f)
#define mieScatter 21e-6f
#define rayleighHeightScale 7994
#define mieHeightScale 1200
#define mieAnisotropy 0.76 // 0.76
#define ozoneAbsorption vec3(0.65e-6, 1.88e-6, 0.085e-6)
#define haze 0.01


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

	// clamp intersection to space in front of origin
	tmin = max(tmin, 0);
	return tmax > 0;
}

bool atmosphereIntersect(vec3 origin, vec3 dir, out float tmin, out float tmax)
{
	float x, y;
	if (!sphereIntersect(origin, dir, atmosphereRadius, x, y))
		return false;

	tmin = x;
	tmax = y;

	if (sphereIntersect(origin, dir, planetRadius, x, y) && x > 0)
		tmax = min(tmax, x);

	return true;
}

vec3 getDensities(float height)
{
	float hr = exp(-height / rayleighHeightScale);
	float hm = exp(-height / mieHeightScale) + haze;
	float ho = max(1 - abs(height - 25e3) / 15e3, 0);
	return vec3(hr, hm, ho);
}

float phase(float cosTheta, float g)
{
	//return 3 / (8 * pi) * ((1 - g * g) * (1 + cosTheta * cosTheta)) / ((2 + g * g) * pow(1 + g * g - 2 * g * cosTheta, 1.5));
	return 1 / (4 * pi) * (1 - g * g) / pow(1 + g * g - 2 * g * cosTheta, 1.5);
}

vec3 sampleTransmittanceLUT(float startingHeight, vec3 dir, vec3 up)
{
	float mu = dot(dir, up);

	vec2 uv;
	uv.x = mu * 0.5 + 0.5;
	//uv.x = sqrt(1 - mu * mu);
	uv.y = startingHeight / (atmosphereRadius - planetRadius);

	return texture(s_transmittanceLUT, uv).rgb;
}

vec3 sampleTransmittance(float startingHeight, vec3 dir, vec3 up)
{
	vec3 origin = vec3(0, planetRadius + startingHeight, 0);

	float mu = dot(dir, up);
	float sinTheta = sqrt(1 - mu * mu);
	dir = normalize(vec3(sinTheta, mu, 0));
	up = vec3(0, 1, 0);

	float tmin, tmax;
	atmosphereIntersect(origin, dir, tmin, tmax);

	int numLightSamples = 64;
	float segmentLength = tmax / numLightSamples;

	vec3 opticalDepth = vec3(0);

	for (int i = 0; i < numLightSamples; i++)
	{
		float t = (i + 0.5) * segmentLength;
		vec3 pos = origin + t * dir;
		float height = length(pos) - planetRadius;

		vec3 density = getDensities(height);
		density *= segmentLength;
		opticalDepth += density;
	}

	vec3 tau = rayleighScatter * opticalDepth.x + mieScatter * opticalDepth.y + ozoneAbsorption * opticalDepth.z;
	vec3 transmittance = exp(-tau);

	return transmittance;
}

vec3 sampleMultiScatter(float height, vec3 toLight, vec3 up)
{
	float mu = dot(up, -toLight);

	vec2 uv;
	uv.x = mu * 0.5 + 0.5;
	uv.y = height / (atmosphereRadius - planetRadius);

	vec3 multi = texture(s_multiScatterLUT, uv).rgb;

	float scatterMultiplier = remap(max(dot(toLight, vec3(0, 1, 0)), 0), 0, 1, 1, 10);
	multi *= scatterMultiplier;

	return multi;
}

vec3 atmosphere(vec3 origin, vec3 dir, vec3 lightDir, float noise, vec3 groundColor)
{
	origin.y += planetRadius;

	float tmin, tmax;
	if (!atmosphereIntersect(origin, dir, tmin, tmax))
		return vec3(0); // space color

	int numSamples = 16;
	float l = tmax - tmin;
	float ldt = 1.0 / numSamples;
	float segmentLength = l * ldt;

	vec3 rayleigh = vec3(0), mie = vec3(0), cloud = vec3(0);
	vec3 opticalDepth = vec3(0);
	vec3 viewTransmittance = vec3(1);

	vec3 toLight = -lightDir;
	float m = dot(dir, toLight);
	float phaseR = 3 / (16 * pi) * (1 + m * m);
	float phaseM = phase(m, mieAnisotropy);

	for (int i = 0; i < numSamples; i++)
	{
		float xi = noise;

		float u0 = (i + xi) * ldt;
		float u1 = (i + 1 + xi) * ldt;
		float u = (i + 0.5 + xi) * ldt;

		float t0 = tmin + l * u0 * u0;
		float t1 = tmin + l * u1 * u1;
		float t = tmin + l * u * u;

		float dt = t1 - t0;

		vec3 pos = origin + t * dir;

		// length(samplePosition) is faster than samplePosition.y ???
		float distanceFromPlanet = length(pos);
		float height = distanceFromPlanet - planetRadius;
		vec3 up = pos / distanceFromPlanet;
		if (height <= 0 && dir.y < 0)
			break;

		vec3 density = getDensities(height);

		vec3 sigmaS = rayleighScatter * density.x + mieScatter * density.y;
		vec3 sigmaT = sigmaS + ozoneAbsorption * density.z;

		vec3 stepTransmittance = exp(-sigmaT * dt);

		opticalDepth += density * dt;
		viewTransmittance = exp(-(rayleighScatter * opticalDepth.x + mieScatter * opticalDepth.y + ozoneAbsorption * opticalDepth.z));

		vec3 lightTransmittance = sampleTransmittanceLUT(height, toLight, up);
		vec3 multiScatter = sampleMultiScatter(height, toLight, up);
		vec3 inscatter = lightTransmittance + multiScatter;

		vec3 transmittance = viewTransmittance * inscatter;

		rayleigh += transmittance * density.x * dt;
		mie += transmittance * density.y * dt;
	}

	float sunIntensity = 25;
	vec3 scattering = (rayleigh * rayleighScatter * phaseR + mie * mieScatter * phaseM) * sunIntensity;

	float tgmin, tgmax;
	if (sphereIntersect(origin, dir, planetRadius, tgmin, tgmax))
	{
		vec3 ground = origin + tgmin * dir;
		vec3 sunToGround = sampleTransmittanceLUT(0, toLight, normalize(ground));
		vec3 albedo = groundColor;
		scattering += viewTransmittance * albedo * sunToGround * sunIntensity * (1 / (4 * pi));
	}

	return scattering;
}

vec4 calculateAerial(vec3 origin, vec3 dir, float maxDistance, vec3 lightDir)
{
	origin.y += planetRadius;

	float tmin, tmax;
	if (!sphereIntersect(origin, dir, atmosphereRadius, tmin, tmax))
		return vec4(0, 0, 0, 1);

	tmax = min(tmax, maxDistance);

	int numSamples = 3;

	float l = tmax - tmin;
	float ldt = 1.0 / numSamples;

	vec3 rayleigh = vec3(0), mie = vec3(0), cloud = vec3(0);
	vec3 opticalDepth = vec3(0);
	vec3 viewTransmittance = vec3(1);

	vec3 toLight = -lightDir;
	float m = dot(dir, toLight);
	float phaseR = 3 / (16 * pi) * (1 + m * m);
	float phaseM = phase(m, mieAnisotropy);

	for (int i = 0; i < numSamples; i++)
	{
		float xi = 0.5;

		float u0 = (i + xi) * ldt;
		float u1 = (i + 1 + xi) * ldt;
		float u = (i + 0.5 + xi) * ldt;

		float t0 = tmin + l * u0 * u0;
		float t1 = tmin + l * u1 * u1;
		float t = tmin + l * u * u;

		float dt = t1 - t0;

		vec3 pos = origin + t * dir;

		float distanceFromPlanet = length(pos);
		float height = distanceFromPlanet - planetRadius;
		vec3 up = pos / distanceFromPlanet;

		vec3 density = getDensities(height);

		opticalDepth += density * dt;
		viewTransmittance = exp(-(rayleighScatter * opticalDepth.x + mieScatter * opticalDepth.y + ozoneAbsorption * opticalDepth.z));

		vec3 lightTransmittance = sampleTransmittanceLUT(height, toLight, up);
		vec3 multiScatter = sampleMultiScatter(height, toLight, up);
		vec3 inscatter = lightTransmittance + multiScatter;

		vec3 transmittance = viewTransmittance * inscatter;

		rayleigh += transmittance * density.x * dt;
		mie += transmittance * density.y * dt;
	}

	float sunIntensity = 25;
	vec3 scattering = (rayleigh * rayleighScatter * phaseR + mie * mieScatter * phaseM) * sunIntensity;

	float T = dot(viewTransmittance, vec3(0.2126, 0.7152, 0.0722));

	return vec4(scattering, T);
}

vec3 attenuateSun(vec3 origin, vec3 sunDirection, float time)
{
	origin.y += planetRadius;

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

		vec3 density = getDensities(height);
		density *= segmentLength;
		
		odr += density.x;
		odm += density.y;
		odo += density.z;
	}

	vec3 tau = rayleighScatter * odr + mieScatter * odm + ozoneAbsorption * odo;
	vec3 attenuation = exp(-tau);

	float sunIntensity = 25;
	return attenuation * sunIntensity;
}
