


static void SubmitMesh(Renderer* renderer, Mesh* mesh, Material* material, SkeletonState* skeleton, const mat4& transform, const mat4& view, const mat4& pv, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer);

void CalculateCascade(vec3 position, quat rotation, float fov, float aspect, float near, float far, int resolution, vec3 lightDir, mat4* lightProjection, mat4* lightView)
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

	float size = max((corners[7] - corners[0]).length(), (corners[3] - corners[0]).length());

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

	//vec3 localMin = vmin - center;
	//vec3 localMax = vmax - center;

	SDL_assert(fabs(vmin.x - center.x) <= 0.5f * size);
	SDL_assert(fabs(vmax.x - center.x) <= 0.5f * size);
	SDL_assert(fabs(vmin.y - center.y) <= 0.5f * size);
	SDL_assert(fabs(vmax.y - center.y) <= 0.5f * size);
	SDL_assert(fabs(vmin.z - center.z) <= 0.5f * size);
	SDL_assert(fabs(vmax.z - center.z) <= 0.5f * size);

	vec3 localMin = vec3(-0.5f * size);
	vec3 localMax = vec3(0.5f * size);

	float texelsPerUnit = resolution / size;
	center *= texelsPerUnit;
	center.xy = floor(center.xy);
	center /= texelsPerUnit;

	//vec3 size = vmax - vmin;
	//vec3 unitsPerTexel = size / (float)directionalLightShadowMapRes;
	//localMin = Vector3.Floor(localMin / unitsPerTexel) * unitsPerTexel;
	//localMax = Vector3.Floor(localMax / unitsPerTexel) * unitsPerTexel;

	vec3 boxPosition = lightRotation * center;

	*lightProjection = mat4::Orthographic(localMin.x, localMax.x, localMin.y, localMax.y, localMin.z, localMax.z);
	*lightView = (mat4::Translate(boxPosition) * mat4::Rotate(lightRotation)).inverted();
}

static void CalculateCascadeMatrices(vec3 position, quat rotation, float fov, float aspect, int resolution, vec3 lightDir, mat4 projections[3], mat4 views[3])
{
	static float NEAR_PLANES[3] = { 0.0f, 15.0f, 60.0f };
	static float FAR_PLANES[3] = { 15.0f, 60.0f, 150.0f };

	for (int i = 0; i < 3; i++)
	{
		CalculateCascade(position, rotation, fov, aspect, NEAR_PLANES[i], FAR_PLANES[i], resolution, lightDir, &projections[i], &views[i]);
	}
}

static void ShadowMapping(Renderer* renderer, vec3 cameraPosition, quat cameraRotation, float near, float fov, float aspect, mat4 projection, mat4 view, mat4 viewInv, vec3 sunDirection)
{
	GPU_SCOPE("Shadow Pass");

	mat4 cascadeProjections[3];
	mat4 cascadeViews[3];
	mat4 cascadePVs[3];

	CalculateCascadeMatrices(cameraPosition, cameraRotation, fov, aspect, SHADOW_MAP_RESOLUTION, sunDirection, cascadeProjections, cascadeViews);

	for (int i = 0; i < 3; i++)
		cascadePVs[i] = cascadeProjections[i] * cascadeViews[i];

	// shadow map
	{
		GPU_TIMER("shadow map");

		for (int cascade = 0; cascade < 3; cascade++)
		{
			vec4 frustumPlanes[6];
			GetFrustumPlanes(cascadePVs[cascade], frustumPlanes);

			SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->shadowMaps[cascade], 0, cmdBuffer);

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->shadowMapPipeline->pipeline);

			for (int i = 0; i < renderer->meshes.size; i++)
			{
				Mesh* mesh = renderer->meshes[i].mesh;
				Material* material = renderer->meshes[i].material;
				const mat4& transform = renderer->meshes[i].transform;
				if (FrustumCulling(mesh->boundingSphere, transform, frustumPlanes))
					SubmitMesh(renderer, mesh, material, nullptr, transform, cascadeViews[cascade], cascadePVs[cascade], renderPass, cmdBuffer);
			}

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->animatedShadowMapPipeline->pipeline);

			for (int i = 0; i < renderer->animatedMeshes.size; i++)
			{
				Mesh* mesh = renderer->animatedMeshes[i].mesh;
				Material* material = renderer->animatedMeshes[i].material;
				SkeletonState* skeleton = renderer->animatedMeshes[i].skeleton;
				const mat4& transform = renderer->animatedMeshes[i].transform;
				if (FrustumCulling(mesh->boundingSphere, transform, frustumPlanes))
					SubmitMesh(renderer, mesh, material, skeleton, transform, cascadeViews[cascade], cascadePVs[cascade], renderPass, cmdBuffer);
			}

			SDL_EndGPURenderPass(renderPass);
		}
	}

	// shadow pass
	{
		GPU_TIMER("shadow buffer");

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->shadowBuffer0, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->shadowPipeline->pipeline);

		struct UniformData
		{
			vec4 params;
			mat4 projection;
			mat4 toLightSpace0;
			mat4 toLightSpace1;
			mat4 toLightSpace2;
		};

		vec3 lightDirection = (view * vec4(sunDirection, 0)).xyz;

		UniformData uniforms = {};
		uniforms.params = vec4(lightDirection, 0);
		uniforms.projection = projection;
		uniforms.toLightSpace0 = cascadePVs[0] * viewInv;
		uniforms.toLightSpace1 = cascadePVs[1] * viewInv;
		uniforms.toLightSpace2 = cascadePVs[2] * viewInv;

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[5];
		gbufferTextures[0] = renderer->gbuffer->depthAttachment;
		gbufferTextures[1] = renderer->gbuffer->colorAttachments[0];
		gbufferTextures[2] = renderer->shadowMaps[0]->depthAttachment;
		gbufferTextures[3] = renderer->shadowMaps[1]->depthAttachment;
		gbufferTextures[4] = renderer->shadowMaps[2]->depthAttachment;

		SDL_GPUSampler* samplers[5];
		samplers[0] = renderer->defaultSampler;
		samplers[1] = renderer->defaultSampler;
		samplers[2] = renderer->shadowSampler;
		samplers[3] = renderer->shadowSampler;
		samplers[4] = renderer->shadowSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 5, gbufferTextures, samplers, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	// blurh
	{
		GPU_TIMER("horizontal blur");

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->shadowBuffer1, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->blurHPipeline->pipeline);

		struct UniformData
		{
			vec4 params;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(near, 0, 0, 0);

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* textures[2];
		textures[0] = renderer->shadowBuffer0->colorAttachments[0];
		textures[1] = renderer->gbuffer->depthAttachment;

		SDL_GPUSampler* samplers[2];
		samplers[0] = renderer->clampedSampler;
		samplers[1] = renderer->clampedSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 2, textures, samplers, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	// blurv
	{
		GPU_TIMER("vertical blur");

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->shadowBuffer0, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->blurVPipeline->pipeline);

		struct UniformData
		{
			vec4 params;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(near, 0, 0, 0);

		SDL_GPUTexture* textures[2];
		textures[0] = renderer->shadowBuffer1->colorAttachments[0];
		textures[1] = renderer->gbuffer->depthAttachment;

		SDL_GPUSampler* samplers[2];
		samplers[0] = renderer->clampedSampler;
		samplers[1] = renderer->clampedSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 2, textures, samplers, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}
}
