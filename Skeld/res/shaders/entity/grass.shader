cull_mode = none;


in(0) vec2 a_position;
in(1) vec4 i_blade;

varying(0) vec3 v_normal;
varying(1) vec3 v_color;
varying(2) float v_roughness;

out(0) vec4 out_normal;
out(1) vec3 out_color;
out(2) vec4 out_material;
out(3) vec4 out_emissive;

uniform(vertex, 0) sampler2D s_heightmap;
uniform(vertex, 1) sampler2D s_normalmap;
uniform(vertex, 2) sampler2D s_coverage;
uniform(vertex, 3) sampler2D s_perlin;

uniform(vertex) {
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
}

uniform(fragment, 0) sampler2D s_diffuse;
uniform(fragment, 1) sampler2D s_roughness;
uniform(fragment, 2) sampler2D s_metallic;

uniform(fragment) {
	vec4 materialData0;
	vec4 materialData1;
	vec4 materialData2;
	vec4 materialData3;

#define hasDiffuse materialData0.x
#define hasRoughness materialData0.y
#define hasMetallic materialData0.z

#define materialColor materialData1.rgb
#define emissiveColor materialData2.rgb
#define emissiveStrength materialData2.a
#define roughnessFactor materialData3.r
#define metallicFactor materialData3.g
}

const float TILE_SIZE = 2.0;
const int TILES = 16;
const int VERTICES = TILES + 1;


float hash12(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v) {
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

vec3 rotateX(vec3 p, float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return vec3(p.x, p.y * c - p.z * s, p.z * c + p.y * s);
}

vec3 rotateY(vec3 p, float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return vec3(p.x * c + p.z * s, p.y, p.z * c - p.x * s);
}

vec4 vertex() {
	vec2 bladePosition = i_blade.xy;
	float rotation = i_blade.z;
	float scale = i_blade.w;

	vec2 heightmapCoord = ((bladePosition / TILE_SIZE) + 0.5) / VERTICES;
	float height = textureLod(s_heightmap, heightmapCoord, 0).x;

	vec2 coverageCoord = bladePosition / TILE_SIZE / TILES;
	float coverage = textureLod(s_coverage, coverageCoord, 0).x;

	float heightPercent = a_position.y;
	float heightPercent2 = heightPercent * heightPercent;
	float heightPercent3 = heightPercent2 * heightPercent;
	float heightPercent4 = heightPercent2 * heightPercent2;

	float curveAmount = heightPercent * hash12(bladePosition) * 0.5;
	vec3 vertexPosition = rotateX(vec3(a_position, 0), curveAmount);

	vertexPosition.x *= 2 * pow(4, u_lod);
	vertexPosition *= scale * coverage * 0.7;

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

	vec3 position = vec3(bladePosition.x, height, bladePosition.y) + vertexPosition;

	vec3 terrainNormal = textureLod(s_normalmap, heightmapCoord, 0).xyz;
	terrainNormal = normalize(vec3(terrainNormal.x, 1, terrainNormal.y));

	vec3 normal = vec3(0, 0, 1);
	normal = rotateX(normal, curveAmount);
	float normalBend = remap(a_position.x, -0.05, 0.05, -0.2 * pi, 0.2 * pi);
	normal = rotateY(normal, rotation + normalBend);

	normal = normalize(mix(normal, terrainNormal, heightPercent2 * 0.7));

	vec4 viewSpaceNormal = u_viewSpaceBuffer > 0.5 ? u_view * vec4(normal, 0) : vec4(normal, 0);
	v_normal = viewSpaceNormal.xyz;

	float specular = heightPercent2 * 0.3;
	v_roughness = 1 - specular;

	vec3 bottomColor = vec3(0.35, 0.5, 0.2) * 1.3;
	vec3 topColor = vec3(0.4, 0.4, 0.3);
	vec3 color = mix(bottomColor, topColor, heightPercent4);

	float ao = mix(0.5, 1.0, heightPercent);
	v_color = color * ao;

	return u_projectionViewModel * vec4(position, 1);
}

void fragment() {
	float roughness = 1; //mix(roughnessFactor, texture(s_roughness, v_texcoord).g, hasRoughness);
	float metallic = 0; //mix(metallicFactor, texture(s_metallic, v_texcoord).b, hasMetallic);

	// dont need this because we're flipping in view space in the vertex shader
	vec3 normal = gl_FrontFacing ? v_normal : -v_normal;

	out_normal = vec4(normal * 0.5 + 0.5, emissiveStrength);
	out_color = v_color;
	out_material = vec4(v_roughness, metallic, 0, 0);
	out_emissive = vec4(linearToSRGB(emissiveColor), 0);
}
