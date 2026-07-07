

const float SH_C0 = 0.282095;
const float SH_C1 = 0.488603;
const float SH_C2 = 1.092548;
const float SH_C3 = 0.315392;
const float SH_C4 = 0.546274;


vec3 sampleSH(vec3 dir)
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

	return distance * dir;
}

float getSampleWeight(vec3 toSample, vec3 size)
{
	float r = length(abs(toSample) / size);
	float weight = exp(-r * 1.0);
	return weight;
}

vec3 getIrradianceSample(vec3 position, vec3 normal, out float weight)
{
	vec3 localPos = position - probePosition;	
	vec3 toSample = parallaxCorrect(localPos, normal, probeSize);
	weight = getSampleWeight(toSample, probeSize);

	vec3 dir = normalize(localPos + toSample);
	return sampleSH(dir);
}

vec3 getIrradiance(vec3 position, vec3 normal)
{
	vec3 worldUp = abs(normal.y) > 0.99 ? vec3(0, 0, 1) : vec3(0, 1, 0); // Avoid gimbal lock

	vec3 tangent = normalize(cross(worldUp, normal));
	vec3 bitangent = cross(normal, tangent);
	
	vec3 sampleRight = normalize(normal + tangent);
	vec3 sampleLeft  = normalize(normal - tangent);
	vec3 sampleFwd   = normalize(normal + bitangent);
	vec3 sampleBack  = normalize(normal - bitangent);

	float weight0, weight1, weight2, weight3, weight4;

	vec3 sample0 = getIrradianceSample(position, normal, weight0);
	vec3 sample1 = getIrradianceSample(position, sampleRight, weight1);
	vec3 sample2 = getIrradianceSample(position, sampleLeft, weight2);
	vec3 sample3 = getIrradianceSample(position, sampleFwd, weight3);
	vec3 sample4 = getIrradianceSample(position, sampleBack, weight4);

	vec3 irradiance = vec3(0);
	irradiance += sample0 * weight0;
	irradiance += sample1 * weight1;
	irradiance += sample2 * weight2;
	irradiance += sample3 * weight3;
	irradiance += sample4 * weight4;

	irradiance /= (weight0 + weight1 + weight2 + weight3 + weight4);

	return irradiance;
}
