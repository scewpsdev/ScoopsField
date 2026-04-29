


void CalculateCascade(vec3 position, quat rotation, float fov, float aspect, float near, float far, vec3 lightDir, mat4* lightProjection, mat4* lightView)
{
	vec3 forward = rotation.forward();
	vec3 up = rotation.up();
	vec3 right = rotation.right();

	float halfHeight = tanf(fov * 0.5f * Deg2Rad);
	float farHalfHeight = far * halfHeight;
	float nearHalfHeight = near * halfHeight;

	float farHalfWidth = farHalfHeight * aspect;
	float nearHalfWidth = nearHalfHeight * aspect;

	vec3 centerFar = position + forward * far;
	vec3 centerNear = position + forward * near;

	vec3 corners[8];
	corners[0] = centerFar + up * farHalfHeight + right * farHalfWidth;
	corners[1] = centerFar + up * farHalfHeight - right * farHalfWidth;
	corners[2] = centerFar - up * farHalfHeight + right * farHalfWidth;
	corners[3] = centerFar - up * farHalfHeight - right * farHalfWidth;
	corners[4] = centerNear + up * nearHalfHeight + right * nearHalfWidth;
	corners[5] = centerNear + up * nearHalfHeight - right * nearHalfWidth;
	corners[6] = centerNear - up * nearHalfHeight + right * nearHalfWidth;
	corners[7] = centerNear - up * nearHalfHeight - right * nearHalfWidth;

	quat lightRotation = quat::LookAt(lightDir, vec3(0, 1, 0));
	quat lightRotationInv = lightRotation.conjugated();

	for (int i = 0; i < 8; i++)
		corners[i] = lightRotationInv * corners[i];

	vec3 vmin = corners[0];
	vec3 vmax = corners[0];
	for (int i = 0; i < 8; i++)
	{
		vmin = min(vmin, corners[i]);
		vmax = max(vmax, corners[i]);
	}

	vec3 center = 0.5f * (vmin + vmax);

	vec3 localMin = vmin - center;
	vec3 localMax = vmax - center;

	//vec3 size = vmax - vmin;
	//vec3 unitsPerTexel = size / (float)directionalLightShadowMapRes;
	//localMin = Vector3.Floor(localMin / unitsPerTexel) * unitsPerTexel;
	//localMax = Vector3.Floor(localMax / unitsPerTexel) * unitsPerTexel;

	vec3 boxPosition = lightRotation * center;

	*lightProjection = mat4::Orthographic(localMin.x, localMax.x, localMin.y, localMax.y, localMin.z, localMax.z);
	*lightView = (mat4::Translate(boxPosition) * mat4::Rotate(lightRotation)).inverted();
}

static void CalculateCascadeMatrices(vec3 position, quat rotation, float fov, float aspect, vec3 lightDir, mat4 projections[3], mat4 views[3])
{
	static float NEAR_PLANES[3] = { 0.0f, 8.0f, 20.0f };
	static float FAR_PLANES[3] = { 8.0f, 20.0f, 150.0f };

	for (int i = 0; i < 3; i++)
	{
		CalculateCascade(position, rotation, fov, aspect, NEAR_PLANES[i], FAR_PLANES[i], lightDir , &projections[i], &views[i]);
	}
}
