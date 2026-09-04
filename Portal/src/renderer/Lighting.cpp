


static bool CullLight(Renderer* renderer, vec3 position, float radius, vec3 portalCorners[4], vec4 viewFrustum[6])
{
	if (!FrustumCulling(position, radius, viewFrustum))
		return false;

	if (portalCorners)
	{
		vec3 R = portalCorners[1] - portalCorners[0];
		vec3 U = portalCorners[3] - portalCorners[0];
		vec3 N = cross(R, U).normalized();

		if (dot(N, position) + radius < 0)
			return false;
	}

	return true;
}

static vec4 CreatePlane(vec3 normal, vec3 point)
{
	normal = normal.normalized();
	return vec4(normal, -dot(normal, point));
}

static void RenderPointLights(Renderer* renderer, SDL_GPURenderPass* renderPass, mat4 pv, mat4 projection, mat4 view, int portalID, vec3 portalCorners[4])
{
	SDL_BindGPUGraphicsPipeline(renderPass, renderer->pointLightPipeline->pipeline);
	SDL_SetGPUStencilReference(renderPass, portalID);

	SDL_GPUBufferBinding vertexBindings[2];
	vertexBindings[0] = {};
	vertexBindings[0].buffer = renderer->cubeVertexBuffer->buffer;
	vertexBindings[0].offset = 0;
	vertexBindings[1] = {};
	vertexBindings[1].buffer = renderer->pointLightInstanceBuffer->buffer;
	vertexBindings[1].offset = 0;

	SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 2);

	SDL_GPUBufferBinding indexBinding = {};
	indexBinding.buffer = renderer->cubeIndexBuffer->buffer;
	indexBinding.offset = 0;

	SDL_BindGPUIndexBuffer(renderPass, &indexBinding, renderer->cubeIndexBuffer->elementSize);

	struct UniformData
	{
		mat4 projection;
		vec4 viewTexel;
	};

	UniformData uniforms = {};
	uniforms.projection = projection;
	uniforms.viewTexel = vec4(1.0f / renderer->hdrTarget->width, 1.0f / renderer->hdrTarget->height, 0, 0);

	SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

	SDL_GPUTextureSamplerBinding textureBindings[4];
	textureBindings[0].texture = renderer->gbuffer->colorAttachments[0];
	textureBindings[1].texture = renderer->gbuffer->colorAttachments[1];
	textureBindings[2].texture = renderer->gbuffer->colorAttachments[2];
	textureBindings[3].texture = renderer->gbuffer->depthAttachment;
	textureBindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
	textureBindings[1].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
	textureBindings[2].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
	textureBindings[3].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];

	SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, 4);

	vec4 frustumPlanes[6];
	GetFrustumPlanes(pv, frustumPlanes);

	for (int i = 0; i < renderer->pointLights.size; i++)
	{
		LightDrawData* pointLight = &renderer->pointLights[i];

		if (!CullLight(renderer, pointLight->position, pointLight->radius, portalCorners, frustumPlanes))
			continue;

		vec3 lightPosition = (view * vec4(pointLight->position, 0)).xyz;

		struct VertexUniformData
		{
			mat4 pv;
			vec4 maskPlanes[5];
			vec3 lightPosition;
			float lightRadius;
			vec4 lightColor;
		};

		VertexUniformData vertexUniforms = {};
		vertexUniforms.pv = pv;
		vertexUniforms.lightPosition = lightPosition;
		vertexUniforms.lightRadius = pointLight->radius;
		vertexUniforms.lightColor = vec4(pointLight->color, 0);

		if (portalCorners)
		{
			vec3 V0 = portalCorners[0];
			vec3 V1 = portalCorners[1];
			vec3 V2 = portalCorners[2];
			vec3 V3 = portalCorners[3];

			vec3 maskNormal = cross(V1 - V0, V3 - V0);
			vertexUniforms.maskPlanes[0] = CreatePlane(maskNormal, V0);
			vertexUniforms.maskPlanes[1] = CreatePlane(cross(V0 - lightPosition, V1 - V0), V0);
			vertexUniforms.maskPlanes[2] = CreatePlane(cross(V1 - lightPosition, V2 - V1), V1);
			vertexUniforms.maskPlanes[3] = CreatePlane(cross(V2 - lightPosition, V3 - V2), V2);
			vertexUniforms.maskPlanes[4] = CreatePlane(cross(V3 - lightPosition, V0 - V3), V3);
		}
		else
		{
			vertexUniforms.maskPlanes[0] = vec4(0, 0, 0, 1);
			vertexUniforms.maskPlanes[1] = vec4(0, 0, 0, 1);
			vertexUniforms.maskPlanes[2] = vec4(0, 0, 0, 1);
			vertexUniforms.maskPlanes[3] = vec4(0, 0, 0, 1);
			vertexUniforms.maskPlanes[4] = vec4(0, 0, 0, 1);
		}

		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &vertexUniforms, sizeof(vertexUniforms));

		SDL_DrawGPUIndexedPrimitives(renderPass, renderer->cubeIndexBuffer->numIndices, 1, 0, 0, 0);
	}
}

static void Lighting(Renderer* renderer, vec3 cameraPosition, float near, mat4 projection, mat4 view, mat4 pv, mat4 projectionInv, mat4 viewInv, mat4 pvInv, vec4 frustumPlanes[6], vec3 sunDirection, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("Lighting");

	// environment light
	if (false)
	{
		GPU_TIMER("environment");

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->environmentLightPipeline->pipeline);

		struct UniformData
		{
			mat4 projectionViewInv;
			mat4 viewInv;
			vec4 params;
		};

		UniformData uniforms = {};
		uniforms.projectionViewInv = pvInv;
		uniforms.viewInv = viewInv;
		uniforms.params = vec4(cameraPosition, 0);

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[6];
		gbufferTextures[0] = renderer->gbuffer->colorAttachments[0];
		gbufferTextures[1] = renderer->gbuffer->colorAttachments[1];
		gbufferTextures[2] = renderer->gbuffer->colorAttachments[2];
		gbufferTextures[3] = renderer->gbuffer->colorAttachments[3];
		gbufferTextures[4] = renderer->gbuffer->depthAttachment;
		gbufferTextures[5] = renderer->skyCubemap->colorAttachments[0];

		SDL_GPUSampler* samplers[6];
		samplers[0] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[1] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[2] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[3] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[4] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[5] = renderer->samplers[TEXTURE_SAMPLER_LINEAR];

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 6, gbufferTextures, samplers, cmdBuffer);
	}

	// reflection probes
	{
		GPU_TIMER("reflection probe");

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->reflectionProbePipeline->pipeline);

		for (int i = 0; i < renderer->reflectionProbes.size; i++)
		{
			// TODO frustum culling

			ReflectionProbe* probe = renderer->reflectionProbes[i].probe;

			struct UniformData
			{
				mat4 projectionViewInv;
				mat4 viewInv;
				vec4 viewTexel;
				vec4 params;
				vec4 params2;
				vec4 params3;
			};

			UniformData uniforms = {};
			uniforms.projectionViewInv = pvInv;
			uniforms.viewInv = viewInv;
			uniforms.viewTexel = vec4(1.0f / renderer->hdrTarget->width, 1.0f / renderer->hdrTarget->height, 0, 0);
			uniforms.params = vec4(probe->position, 0);
			uniforms.params2 = vec4(probe->size, 0);
			uniforms.params3 = vec4(cameraPosition, 0);

			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTextureSamplerBinding textureBindings[6];

			textureBindings[0].texture = renderer->gbuffer->colorAttachments[0];
			textureBindings[1].texture = renderer->gbuffer->colorAttachments[1];
			textureBindings[2].texture = renderer->gbuffer->colorAttachments[2];
			textureBindings[3].texture = renderer->gbuffer->depthAttachment;
			textureBindings[4].texture = probe->specular;
			textureBindings[5].texture = renderer->brdfLUT->handle;

			textureBindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
			textureBindings[1].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
			textureBindings[2].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
			textureBindings[3].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
			textureBindings[4].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR];
			textureBindings[5].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];

			SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, 6);

			SDL_BindGPUFragmentStorageBuffers(renderPass, 0, &probe->irradiance, 1);

			if (IsInBounds(cameraPosition, probe->position - probe->size - 0.999f, probe->position + probe->size + 0.999f))
			{
				SDL_GPUBufferBinding bufferBindings[1];
				bufferBindings[0].buffer = renderer->screenQuad.vertexBuffer->buffer;
				bufferBindings[0].offset = 0;

				SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);

				struct VertexUniformData
				{
					mat4 projectionView;
					vec4 params;
					vec4 params2;
				};

				VertexUniformData vertexUniforms = {};
				vertexUniforms.projectionView = mat4::Identity;
				vertexUniforms.params = vec4(0, 0, 1, 0);
				vertexUniforms.params2 = vec4(0.001f, 0.001f, 0.001f, 1);

				SDL_PushGPUVertexUniformData(cmdBuffer, 0, &vertexUniforms, sizeof(vertexUniforms));

				SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
			}
			else
			{
				SDL_GPUBufferBinding vertexBindings[1];
				vertexBindings[0] = {};
				vertexBindings[0].buffer = renderer->cubeVertexBuffer->buffer;
				vertexBindings[0].offset = 0;

				SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 1);

				SDL_GPUBufferBinding indexBinding = {};
				indexBinding.buffer = renderer->cubeIndexBuffer->buffer;
				indexBinding.offset = 0;

				SDL_BindGPUIndexBuffer(renderPass, &indexBinding, renderer->cubeIndexBuffer->elementSize);

				struct VertexUniformData
				{
					mat4 projectionView;
					vec4 params;
					vec4 params2;
				};

				VertexUniformData vertexUniforms = {};
				vertexUniforms.projectionView = pv;
				vertexUniforms.params = vec4(probe->position, 0);
				vertexUniforms.params2 = vec4(probe->size, 0);

				SDL_PushGPUVertexUniformData(cmdBuffer, 0, &vertexUniforms, sizeof(vertexUniforms));

				SDL_DrawGPUIndexedPrimitives(renderPass, renderer->cubeIndexBuffer->numIndices, 1, 0, 0, 0);
			}
		}
	}

	// directional lights
	if (false)
	{
		GPU_TIMER("sun");

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->directionalLightPipeline->pipeline);

		vec3 lightDirection = (view * vec4(sunDirection, 0)).xyz;

		struct UniformData
		{
			vec4 lightDirection;
			mat4 projection;
		};

		UniformData uniforms = {};
		uniforms.lightDirection = vec4(lightDirection, 0);
		uniforms.projection = projection;

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[6];
		gbufferTextures[0] = renderer->gbuffer->colorAttachments[0];
		gbufferTextures[1] = renderer->gbuffer->colorAttachments[1];
		gbufferTextures[2] = renderer->gbuffer->colorAttachments[2];
		gbufferTextures[3] = renderer->gbuffer->depthAttachment;
		gbufferTextures[4] = renderer->sunColorBuffer;
		gbufferTextures[5] = renderer->shadowBuffer0->colorAttachments[0];

		SDL_GPUSampler* samplers[6];
		samplers[0] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[1] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[2] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[3] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		samplers[4] = renderer->samplers[TEXTURE_SAMPLER_CLAMPED];
		samplers[5] = renderer->samplers[TEXTURE_SAMPLER_CLAMPED];

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 6, gbufferTextures, samplers, cmdBuffer);
	}

	// point lights
	if (renderer->pointLights.size)
	{
		GPU_TIMER("point lights");

		RenderPointLights(renderer, renderPass, pv, projection, view, 0, nullptr);

		// lights shining out from a portal
		for (int i = 0; i < renderer->portals.size; i++)
		{
			PortalDrawData* portal = &renderer->portals[i];
			vec3 position = portal->transform.translation();
			vec3 right = portal->transform.rotation().right();
			vec3 up = portal->transform.rotation().up();
			vec3 corners[4] = {
				view * (position - right - up),
				view * (position + right - up),
				view * (position + right + up),
				view * (position - right + up),
			};
			RenderPointLights(renderer, renderPass, pv * portal->portalView, projection, view * portal->portalView, 0, corners);
		}

		for (int i = 0; i < renderer->portals.size; i++)
		{
			PortalDrawData* portal = &renderer->portals[i];
			RenderPointLights(renderer, renderPass, pv * portal->portalView, projection, view * portal->portalView, portal->portalID, nullptr);

			// lights shining into a portal

			vec3 position = portal->transform.translation();
			vec3 right = portal->transform.rotation().right();
			vec3 up = portal->transform.rotation().up();
			vec3 corners[4] = {
				view * (position + right - up),
				view * (position - right - up),
				view * (position - right + up),
				view * (position + right + up),
			};

			RenderPointLights(renderer, renderPass, pv, projection, view, portal->portalID, corners);
		}
	}
}
