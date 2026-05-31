


#define pi 3.14159265359


float remap(float f, float min, float max, float newMin, float newMax)
{
	return (f - min) / (max - min) * (newMax - newMin) + newMin;
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

vec3 linearToSRGB(vec3 color)
{
	float gamma = 2.2;
	return pow(color, vec3(1.0 / gamma));
}

float RGBToLuminance(vec3 rgb)
{
	return dot(rgb, vec3(0.2126, 0.7152, 0.0722));
}
