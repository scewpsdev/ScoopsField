
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

// Variant fresnel equation taking the roughness into account;
vec3 fresnel2(float hdotv, vec3 f0, float roughness)
{
	//return f0 + (1.0 - f0) * pow(clamp(1.0 - hdotv, 0.0, 1.0), 5.0);
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - hdotv, 5.0);
}

// Directional light indirect specular lighting
vec3 directionalLight(vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, vec3 lightDirection, vec3 lightColor)
{
	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 fLambert = albedo / PI;

	// Per light radiance
	vec3 wi = -lightDirection;
	vec3 h = normalize(view + wi);

	// Cook-Torrance BRDF
	float d = normalDistribution(normal, h, roughness);
	float g = geometrySmith(normal, view, wi, roughness);
	vec3 f = fresnel2(max(dot(h, view), 0.0), f0, roughness);
	vec3 numerator = d * f * g;
	float denominator = 4.0 * max(dot(view, normal), 0.0) * max(dot(wi, normal), 0.0);
	vec3 specular = numerator / max(denominator, 0.0001);

	vec3 ks = f;
	vec3 kd = (1.0 - ks) * (1.0 - metallic);

	vec3 radiance = lightColor;

	float ndotwi = max(dot(wi, normal), 0.0);
	float shadow = 1.0; // Shadow mapping

	vec3 s = (specular + fLambert * kd) * radiance * ndotwi * shadow;

	return s;
}

// Radiance calculation for radial flux over the angle w
vec3 L(vec3 color, float dist)
{
	float attenuation = 1.0 / (1.0 + 1 * dist + 2 * dist * dist);

	vec3 radiance = color * attenuation;

	//float maxComponent = max(radiance.r, max(radiance.g, radiance.b));
	//radiance *= max(1 - 0.01 / maxComponent, 0);

	return radiance;
}

// Point light indirect specular lighting
vec3 pointLight(vec3 position, vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, vec3 lightPosition, vec3 lightColor)
{
	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 fLambert = albedo / PI;

	// Per light radiance
	vec3 toLight = lightPosition - position;
	vec3 wi = normalize(toLight);
	vec3 h = normalize(view + wi);

	float distanceSq = dot(toLight, toLight);
	vec3 radiance = L(lightColor, distanceSq);

	// Cook-Torrance BRDF
	float d = normalDistribution(normal, h, roughness);
	float g = geometrySmith(normal, view, wi, roughness);
	vec3 f = fresnel2(max(dot(h, view), 0.0), f0, roughness);
	vec3 numerator = d * f * g;
	float denominator = 4.0 * max(dot(view, normal), 0.0) * max(dot(wi, normal), 0.0);
	vec3 specular = numerator / max(denominator, 0.0001);

	vec3 ks = f;
	vec3 kd = (1.0 - ks) * (1.0 - metallic);

	float ndotwi = max(dot(wi, normal), 0.0);
	float shadow = 1.0; // Shadow mapping

	vec3 s = (specular + fLambert * kd) * radiance * ndotwi * shadow;

	return s;
}

vec3 sampleEnvironmentIrradiance(vec3 normal, samplerCube environmentMap, float environmentIntensity)
{
	return textureLod(environmentMap, normal, log2(textureSize(environmentMap, 0).x)).rgb * environmentIntensity;
}

vec3 sampleEnvironmentPrefiltered(vec3 normal, vec3 view, float roughness, samplerCube environmentMap, float environmentIntensity)
{
	vec3 r = reflect(-view, normal);
	float lodFactor = 1.0 - exp(-roughness * 12);

	return textureLod(environmentMap, r, lodFactor * log2(textureSize(environmentMap, 0).x)).rgb * environmentIntensity;
}

vec3 environmentLight(vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, samplerCube environmentMap, float environmentIntensity)
{
	vec3 irradiance = sampleEnvironmentIrradiance(normal, environmentMap, environmentIntensity);

	vec3 diffuse = irradiance * albedo;

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 ks = fresnel2(max(dot(normal, view), 0.0), f0, roughness);
	vec3 kd = (1.0 - ks) * (1.0 - metallic);

	vec3 prefiltered = sampleEnvironmentPrefiltered(normal, view, roughness, environmentMap, environmentIntensity);

	vec2 brdfInteg = vec2(1.0, 0.0);
	vec3 specular = prefiltered * (ks * brdfInteg.r + brdfInteg.g);

	vec3 ambient = kd * diffuse + specular;

	return ambient;
}
