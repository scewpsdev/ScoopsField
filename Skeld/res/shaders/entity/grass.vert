#version 460

#include "../common.glsl"

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 i_blade;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec3 v_color;
layout (location = 2) out float v_roughness;


layout(set = 0, binding = 0) uniform sampler2D s_heightmap;
layout(set = 0, binding = 1) uniform sampler2D s_normalmap;
layout(set = 0, binding = 2) uniform sampler2D s_perlin;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_view;
	mat4 u_projection;
	mat4 u_model;
	vec4 params;

	vec4 grassData;

#define u_time params.x
#define u_viewSpaceBuffer params.y

#define u_terrainPosition grassData.xy
#define u_lod grassData.z
};


#define TILE_SIZE 2.0
#define VERTICES 17


float hash12(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v)
{
	  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
	           -0.577350269189626, 0.024390243902439);
	  vec2 i  = floor(v + dot(v, C.yy) );
	  vec2 x0 = v -   i + dot(i, C.xx);
	  vec2 i1;
	  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	  vec4 x12 = x0.xyxy + C.xxzz;
	  x12.xy -= i1;
	  i = mod(i, 289.0);
	  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
	  + i.x + vec3(0.0, i1.x, 1.0 ));
	  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
	    dot(x12.zw,x12.zw)), 0.0);
	  m = m*m ;
	  m = m*m ;
	  vec3 x = 2.0 * fract(p * C.www) - 1.0;
	  vec3 h = abs(x) - 0.5;
	  vec3 ox = floor(x + 0.5);
	  vec3 a0 = x - ox;
	  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
	  vec3 g;
	  g.x  = a0.x  * x0.x  + h.x  * x0.y;
	  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	  return 130.0 * dot(m, g);
}

// Cheap 2D value/gradient noise approximation
float cheap_noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
	
    // Smoothstep creates the smooth S-curve interpolation (3t^2 - 2t^3)
    vec2 u = f * f * (3.0 - 2.0 * f);

    // Mix (bilinear interpolation) between the 4 corners of the grid cell
    return mix(mix(hash12(i + vec2(0.0, 0.0)), hash12(i + vec2(1.0, 0.0)), u.x),
               mix(hash12(i + vec2(0.0, 1.0)), hash12(i + vec2(1.0, 1.0)), u.x), u.y);
}

vec3 rotateX(vec3 p, float angle)
{
	float s = sin(angle);
	float c = cos(angle);
	return vec3(p.x, p.y * c - p.z * s, p.z * c + p.y * s);
}

vec3 rotateY(vec3 p, float angle)
{
	float s = sin(angle);
	float c = cos(angle);
	return vec3(p.x * c + p.z * s, p.y, p.z * c - p.x * s);
}

void main()
{
	vec2 bladePosition = i_blade.xy;
	float rotation = i_blade.z;
	float scale = i_blade.w;

	vec2 heightmapCoord = ((bladePosition / TILE_SIZE) + 0.5) / VERTICES;
	float height = textureLod(s_heightmap, heightmapCoord, 0).x;

	float heightPercent = a_position.y;
	float heightPercent2 = heightPercent * heightPercent;
	float heightPercent3 = heightPercent2 * heightPercent;
	float heightPercent4 = heightPercent2 * heightPercent2;

	float curveAmount = heightPercent * hash12(bladePosition) * 0.5;
	vec3 vertexPosition = rotateX(vec3(a_position, 0), curveAmount);

	vertexPosition.x *= 2 * pow(4, u_lod);

	vertexPosition = rotateY(vertexPosition, rotation);

	// wind
	vec2 worldxz = u_terrainPosition + bladePosition;
	float windNoise1 = snoise(worldxz * 0.25 + u_time * 0.3);
	float windNoise2 = snoise(worldxz * 0.25 + (u_time - 0.3) * 0.3);
	float windNoise = mix(windNoise1, windNoise2, heightPercent2);
	float windLean = remap(windNoise, -1, 1, 0, 1) * heightPercent;
	float windDir = snoise(-worldxz * 0.02 + 0.02 * u_time) * pi;
	
	vertexPosition = rotateY(vertexPosition, -windDir);
	vertexPosition = rotateX(vertexPosition, windLean);
	vertexPosition = rotateY(vertexPosition, windDir);

	vertexPosition *= scale * 0.7;

	// view space thiccen
	/*
	vec3 view = -u_view[2].xyz;
	vec3 right = u_view[0].xyz;
	vec3 bladeDirection = rotateY(vec3(0, 0, 1), -rotation); // why does - work???? the heck
	float VdotN = abs(dot(normalize(view.xz), bladeDirection.xz));
	float thiccness = easeOut(1 - VdotN) * smoothstep(0, 0.2, VdotN);
	vertexPosition.x *= 1 + thiccness * 0.0;
	*/

	vec3 position = vec3(bladePosition.x, height, bladePosition.y) + vertexPosition;

	vec3 terrainNormal = textureLod(s_normalmap, heightmapCoord, 0).xyz;
	terrainNormal = normalize(vec3(terrainNormal.x, 1, terrainNormal.y));

	vec3 normal = vec3(0, 0, 1);
	normal = rotateX(normal, curveAmount);
	float normalBend = remap(a_position.x, -0.05, 0.05, -0.2 * pi, 0.2 * pi);
	normal = rotateY(normal, rotation + normalBend);

	vec3 view = -u_view[2].xyz;
	//if (dot(view, normal) > 0) normal *= -1;

	normal = normalize(mix(normal, terrainNormal, heightPercent2 * 0.7));

	vec4 viewSpaceNormal = u_viewSpaceBuffer > 0.5 ? u_view * vec4(normal, 0) : vec4(normal, 0);
	v_normal = viewSpaceNormal.xyz;

	float specular = heightPercent2 * 0.3;
	v_roughness = 1 - specular;
	//v_roughness = VdotN;

	vec3 bottomColor = vec3(0.35, 0.5, 0.2) * 1.3;
	vec3 topColor = vec3(0.4, 0.4, 0.3);
	vec3 color = mix(bottomColor, topColor, heightPercent4);

	float ao = mix(0.5, 1.0, heightPercent);
	v_color = color * ao;

	/*
	float heightMask = max(a_position.y, 0) * 0.25;
	heightMask = heightMask * heightMask;
	vec3 windDirection = vec3(0.7, 0, 0.7);

	float trunkWave = sin(u_time * 0.7 + (a_position.x + a_position.z) * 0.02);
	worldPosition.xyz += windDirection * trunkWave * heightMask * 0.1;

	float wave1 = sin(u_time * 1.5 + (a_position.x + a_position.z) * 0.5);
	float wave2 = cos(u_time * 1.5 * 1.8 + (a_position.x - a_position.z) * 0.5 * 1.5);
	float wind = wave1 * 0.7 + wave2 * 0.3;
	float leafMask = min(a_position.x * a_position.x + a_position.z * a_position.z, 1);
	worldPosition.xyz += windDirection * wind * heightMask * leafMask * 0.05;

	float leafFlutter = sin(u_time * 10.0 + a_position.x * 5.0);
	float flutterMask = min((a_position.x * a_position.x + a_position.z * a_position.z) * 0.2, 1);
    worldPosition.xyz += leafFlutter * heightMask * flutterMask * 0.0025;
	*/

	gl_Position = u_projectionViewModel * vec4(position, 1);
}
