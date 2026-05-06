#version 460

#include "../common.glsl"
#include "lighting.glsl"

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;
layout(set = 2, binding = 4) uniform samplerCube s_cubemap;

layout(set = 2, binding = 5) readonly buffer SHBuffer {
	vec3 coefficients[9];
};

layout(std140, set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionViewInv;
	mat4 viewInv;
	vec4 viewTexel;
	vec4 params;
	vec4 params2;
	vec4 params3;

#define probePosition params.xyz
#define probeSize params2.xyz
#define cameraPosition params3.xyz
};


const float SH_C0 = 0.282095;
const float SH_C1 = 0.488603;
const float SH_C2 = 1.092548;
const float SH_C3 = 0.315392;
const float SH_C4 = 0.546274;


vec3 getIrradiance(vec3 dir)
{
	 return (
        coefficients[0] * SH_C0 +

        coefficients[1] * SH_C1 * dir.y +
        coefficients[2] * SH_C1 * dir.z +
        coefficients[3] * SH_C1 * dir.x +

        coefficients[4] * SH_C2 * (dir.x * dir.y) +
        coefficients[5] * SH_C2 * (dir.y * dir.z) +
        coefficients[6] * SH_C3 * (3 * dir.z * dir.z - 1) +
        coefficients[7] * SH_C2 * (dir.x * dir.z) +
        coefficients[8] * SH_C4 * (dir.x * dir.x - dir.y * dir.y)
    );
}


vec3 parallaxCorrect(vec3 position, vec3 dir, vec3 size)
{
	vec3 boxMin = -size;
	vec3 boxMax = size;

	position = clamp(position, boxMin + 0.1, boxMax - 0.1);

	vec3 firstPlaneIntersect = (boxMax - position) / dir;
	vec3 secondPlaneIntersect = (boxMin - position) / dir;

	vec3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);
	float distance = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	distance = abs(distance);

	return position + dir * distance;
}

vec3 getBentNormal(vec3 position, vec3 normal, vec3 size)
{
	vec3 boxMin = -size;
	vec3 boxMax = size;

	 // 1. Calculate distance to each wall [0.0 = on the wall, 1.0 = center/far]
    // Normalized distance: 0 at wall, 1 at center
    vec3 distToMax = boxMax - position;
    vec3 distToMin = position - boxMin;
    
    // 2. Identify which wall we are closest to
    // 'closeness' is 1.0 when touching a wall, 0.0 when far away
    float falloff = 0.5; // Controls how far from the wall the "bending" starts
    vec3 closenessMax = clamp(1.0 - (distToMax / falloff), 0.0, 1.0);
    vec3 closenessMin = clamp(1.0 - (distToMin / falloff), 0.0, 1.0);
    
    // 3. Create a vector pointing AWAY from the nearest walls
    // If close to +X wall, bend toward -X
    vec3 bendDir = vec3(0.0);
    bendDir.x = closenessMin.x - closenessMax.x; 
    bendDir.y = closenessMin.y - closenessMax.y;
    bendDir.z = closenessMin.z - closenessMax.z;

    // 4. Mix the original normal with the bend direction
    // Strength 0.5 means at the very corner, the normal is 50% geometry, 50% bent
    float bendStrength = 0.5; 
    vec3 bentN = normalize(mix(normal, normalize(bendDir + normal), bendStrength));
    
    return bentN;
}

vec3 sampleEnvironmentIrradiance(vec3 position, vec3 normal, samplerCube environmentMap)
{
	/*
	vec3 worldUp = abs(normal.y) > 0.99 ? vec3(0, 0, 1) : vec3(0, 1, 0); // Avoid gimbal lock

	vec3 tangent = normalize(cross(worldUp, normal));
	vec3 bitangent = cross(normal, tangent);
	
	// Your 4 side directions
	vec3 sampleRight = normalize(normal + tangent);
	vec3 sampleLeft  = normalize(normal - tangent);
	vec3 sampleFwd   = normalize(normal + bitangent);
	vec3 sampleBack  = normalize(normal - bitangent);
	*/

	vec3 dir = parallaxCorrect(position - probePosition, normal, probeSize);
	return textureLod(environmentMap, dir * vec3(1, 1, -1), log2(textureSize(environmentMap, 0).x)).rgb;
}

vec3 sampleEnvironmentPrefiltered(vec3 position, vec3 normal, vec3 view, float roughness, samplerCube environmentMap)
{
	vec3 r = reflect(-view, normal);
	float lodFactor = roughness; //1.0 - exp(-roughness * 12);

	vec3 dir = parallaxCorrect(position - probePosition, r, probeSize);

	return textureLod(environmentMap, dir * vec3(1, 1, -1), lodFactor * log2(textureSize(environmentMap, 0).x)).rgb;
}

vec3 environmentLight(vec3 position, vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, samplerCube environmentMap)
{
	vec3 bentNormal = getBentNormal(position - probePosition, normal, probeSize);
	vec3 dir = normalize(parallaxCorrect(position - probePosition, bentNormal, probeSize));
	vec3 irradiance = getIrradiance(dir); //sampleEnvironmentIrradiance(position, normal, environmentMap);

	vec3 diffuse = irradiance * albedo;

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 ks = fresnel2(max(dot(normal, view), 0.0), f0, roughness);
	vec3 kd = (1.0 - ks) * (1.0 - metallic);

	vec3 prefiltered = sampleEnvironmentPrefiltered(position, normal, view, roughness, environmentMap);

	vec2 brdfInteg = vec2(1.0, 0.0);
	vec3 specular = prefiltered * (ks * brdfInteg.r + brdfInteg.g);

	vec3 ambient = kd * diffuse + specular;

	return ambient;
}

vec3 reconstructPosition(vec2 uv, float depth)
{
	vec4 ndc = vec4(uv.x * 2 - 1, uv.y * -2 + 1, depth, 1);
	vec4 worldPosition = projectionViewInv * ndc;
	return worldPosition.xyz / worldPosition.w;
}

void main()
{
	vec2 uv = gl_FragCoord.xy * viewTexel.xy;
	float depth = texture(s_depth, uv).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(uv, depth); // world space position
	vec3 view = normalize(cameraPosition - position); // world space view

	vec3 viewSpaceNormal = texture(s_normal, uv).rgb * 2 - 1;
	vec3 normal = (viewInv * vec4(viewSpaceNormal, 0)).xyz; // world space normal
	vec3 albedo = texture(s_color, uv).rgb;

	vec4 material = texture(s_material, uv);
	float roughness = material.r;
	float metallic = material.g;

	vec3 radiance = environmentLight(position, normal, view, albedo, roughness, metallic, s_cubemap);

	float sdf = length(max(abs(position - probePosition) - probeSize, 0));
	const float maxDistance = 1.0;
	float alpha = max(remap(sdf, 0, maxDistance, 1, 0), 0);

	out_color = vec4(radiance, alpha);
}
