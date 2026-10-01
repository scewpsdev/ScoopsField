
#define PI 3.14159265359


// Simulates microfacet model (Trowbridge-Reitz GGX)
float normalDistribution(vec3 normal, vec3 h, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float ndoth = max(dot(normal, h), 0.0);

	float denom = ndoth * ndoth * (a2 - 1.0) + 1.0;

	return a2 / (PI * denom * denom);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = NdotH2 * (a2 - 1) + 1;
	denom = PI * denom * denom;

	return num / max(denom, 0.0000001);
}

// Self shadowing of microfacets (Schlick-GGX)
float geometryGGX(float ndotv, float k)
{
	return ndotv / (ndotv * (1.0 - k) + k);
}

float geometrySmith(vec3 normal, vec3 view, vec3 wi, float roughness)
{
	// Roughness remapping
	float r = roughness + 1.0;
	float k = r * r / 8.0;

	float ndotv = max(dot(normal, view), 0.0); // TODO precalculate this
	float ndotl = max(dot(normal, wi), 0.0); // TODO precalculate this

	return geometryGGX(ndotv, k) * geometryGGX(ndotl, k);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1;
	float k = r * r / 8;

	return NdotV / (NdotV * (1 - k) + k);
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// Variant fresnel equation taking the roughness into account;
vec3 fresnel2(float hdotv, vec3 f0, float roughness)
{
	//return f0 + (1.0 - f0) * pow(clamp(1.0 - hdotv, 0.0, 1.0), 5.0);
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - hdotv, 5.0);
}

vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	return F0 + (1 - F0) * pow(clamp(1 - HdotV, 0, 1), 5);
}

float calculateLightRadius(vec3 color)
{
	float maxChannel = max(color.r, max(color.g, color.b));
	float threshold = 0.001;
	float radiusSquared = maxChannel / threshold;
	return radiusSquared;
}

// Radiance calculation for radial flux over the angle w
vec3 L(vec3 color, float distanceSquared, float areaRadiusSquared)
{
	float attenuation = 1.0 / (distanceSquared + areaRadiusSquared);
	float radiusSquared = calculateLightRadius(color);
	float d2 = distanceSquared / radiusSquared;
	float windowing = max(0, 1 - d2 * d2);
	attenuation *= windowing * windowing;

	vec3 radiance = color * attenuation;

	//float maxComponent = max(radiance.r, max(radiance.g, radiance.b));
	//radiance *= max(1 - 0.01 / maxComponent, 0);

	return radiance;
}
