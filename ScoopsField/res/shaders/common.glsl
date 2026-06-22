


#define pi 3.14159265359


float remap(float f, float min, float max, float newMin, float newMax)
{
	return (f - min) / (max - min) * (newMax - newMin) + newMin;
}

float remap(float f, float a, float b)
{
	return (f - a) / (b - a);
}

float linearstep(float edge0, float edge1, float x)
{
	return clamp((x - edge0) / (edge1 - edge0), 0, 1);
}

float depthToDistance(float depth, float near, float far)
{
	depth = depth * 2 - 1;
	return 2.0 * near * far / (far + near - depth * (far - near));
}

float distanceToDepth(float distance, float near, float far)
{
	float a = -(far + near) / (far - near);
	float b = -2.0 * far * near / (far - near);
	float depth = (-a * distance + b) / distance;
	depth = depth * 0.5 + 0.5;
	return depth;
}

vec3 SRGBToLinear(vec3 color)
{
	float gamma = 2.2;
	return pow(color, vec3(gamma));
}

vec4 SRGBToLinear(vec4 color)
{
	float gamma = 2.2;
	return vec4(pow(color.rgb, vec3(gamma)), color.a);
}

vec3 linearToSRGB(vec3 color)
{
	float gamma = 2.2;
	return pow(color, vec3(1.0 / gamma));
}

vec4 linearToSRGB(vec4 color)
{
	float gamma = 2.2;
	return vec4(pow(color.rgb, vec3(1.0 / gamma)), color.a);
}

float RGBToLuminance(vec3 rgb)
{
	return dot(rgb, vec3(0.2126, 0.7152, 0.0722));
}
