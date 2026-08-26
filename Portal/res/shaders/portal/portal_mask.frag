#version 460

layout(set = 3, binding = 0) uniform UniformBlock {
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
};


void main()
{
}
