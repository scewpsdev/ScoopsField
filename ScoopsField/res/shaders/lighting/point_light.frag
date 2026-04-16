#version 460

#include "lighting.glsl"

layout (location = 0) in vec3 v_lightPosition;
layout (location = 1) in vec3 v_lightColor;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;

layout(std140, set = 3, binding = 0) uniform UniformBlock {
    mat4 projection;
	vec4 u_viewTexel;
};


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

vec3 reconstructPosition(vec2 uv, float depth)
{
	vec2 ndc = vec2(uv.x * 2 - 1, 1 - uv.y * 2);
	float near = projection[3][2];
	float x = projection[0][0];
	float y = projection[1][1];

	vec3 view;
	view.z = near / depth;
	view.x = ndc.x * view.z / x;
	view.y = ndc.y * view.z / y;
	view.z *= -1; // right handed coordinate system
	return view;
}

void main()
{
	vec2 uv = gl_FragCoord.xy * u_viewTexel.xy;
	float depth = texture(s_depth, uv).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(uv, depth);
	vec3 view = normalize(-position);

	vec3 normal = texture(s_normal, uv).rgb;
	vec3 albedo = texture(s_color, uv).rgb;

	vec4 material = texture(s_material, uv);
	float roughness = material.r;
	float metallic = material.g;

	vec3 radiance = pointLight(position, normal, view, albedo, roughness, metallic, v_lightPosition, v_lightColor);
	
	out_color = vec4(radiance, 1);
}
