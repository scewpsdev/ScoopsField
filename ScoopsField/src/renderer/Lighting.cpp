


static void Lighting(Renderer* renderer, vec3 cameraPosition, float near, mat4 projection, mat4 view, mat4 pv, mat4 projectionInv, mat4 viewInv, mat4 pvInv, vec4 frustumPlanes[6], vec3 sunDirection, mat4 cascadePVs[3], SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("lighting");

	// environment light
	{
		GPU_SCOPE("environment");

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->environmentLightPipeline->pipeline);

		float environmentIntensity = 1.0f;

		struct UniformData
		{
			vec4 params;
			mat4 projectionViewInv;
			mat4 viewInv;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(cameraPosition, environmentIntensity);
		uniforms.projectionViewInv = pvInv;
		uniforms.viewInv = viewInv;

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[MAX_COLOR_ATTACHMENTS + 2];
		for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			gbufferTextures[i] = renderer->gbuffer->colorAttachments[i];
		gbufferTextures[renderer->gbuffer->numColorAttachments] = renderer->gbuffer->depthAttachment;
		gbufferTextures[renderer->gbuffer->numColorAttachments + 1] = renderer->skyCubemap->colorAttachments[0];

		SDL_GPUSampler* samplers[MAX_COLOR_ATTACHMENTS + 2];
		for (int i = 0; i < MAX_COLOR_ATTACHMENTS + 2; i++)
			samplers[i] = renderer->defaultSampler;
		samplers[renderer->gbuffer->numColorAttachments + 1] = renderer->linearSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, renderer->gbuffer->numColorAttachments + 2, gbufferTextures, samplers, cmdBuffer);
	}

	// directional lights
	{
		GPU_SCOPE("sun");

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->directionalLightPipeline->pipeline);

		vec3 lightDirection = (view * vec4(sunDirection, 0)).xyz;

		vec3 lightColor = vec3(1, 1, 1);
		float lightIntensity = 10;
		lightColor *= lightIntensity;

		struct UniformData
		{
			vec4 lightDirection;
			vec4 lightColor;
			mat4 projection;
			mat4 toLightSpace0;
			mat4 toLightSpace1;
			mat4 toLightSpace2;
		};

		UniformData uniforms = {};
		uniforms.lightDirection = vec4(lightDirection, gameTime);
		uniforms.lightColor = vec4(lightColor, 0);
		uniforms.projection = projection;
		uniforms.toLightSpace0 = cascadePVs[0] * viewInv;
		uniforms.toLightSpace1 = cascadePVs[1] * viewInv;
		uniforms.toLightSpace2 = cascadePVs[2] * viewInv;

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[8];
		for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			gbufferTextures[i] = renderer->gbuffer->colorAttachments[i];
		gbufferTextures[renderer->gbuffer->numColorAttachments] = renderer->gbuffer->depthAttachment;
		gbufferTextures[renderer->gbuffer->numColorAttachments + 1] = renderer->sunColorBuffer;
		gbufferTextures[renderer->gbuffer->numColorAttachments + 2] = renderer->shadowMaps[0]->depthAttachment;
		gbufferTextures[renderer->gbuffer->numColorAttachments + 3] = renderer->shadowMaps[1]->depthAttachment;
		gbufferTextures[renderer->gbuffer->numColorAttachments + 4] = renderer->shadowMaps[2]->depthAttachment;

		SDL_GPUSampler* samplers[8];
		for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			samplers[i] = renderer->defaultSampler;
		samplers[renderer->gbuffer->numColorAttachments] = renderer->defaultSampler;
		samplers[renderer->gbuffer->numColorAttachments + 1] = renderer->defaultSampler;
		samplers[renderer->gbuffer->numColorAttachments + 2] = renderer->shadowSampler;
		samplers[renderer->gbuffer->numColorAttachments + 3] = renderer->shadowSampler;
		samplers[renderer->gbuffer->numColorAttachments + 4] = renderer->shadowSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, renderer->gbuffer->numColorAttachments + 5, gbufferTextures, samplers, cmdBuffer);
	}

	// point lights
	{
		GPU_SCOPE("point lights");

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->pointLightPipeline->pipeline);

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

		struct VertexUniformData
		{
			mat4 pv;
			mat4 view;
		};

		VertexUniformData vertexUniforms = {};
		vertexUniforms.pv = pv;
		vertexUniforms.view = view;

		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &vertexUniforms, sizeof(vertexUniforms));

		struct UniformData
		{
			mat4 projection;
			vec4 viewTexel;
		};

		UniformData uniforms = {};
		uniforms.projection = projection;
		uniforms.viewTexel = vec4(1.0f / renderer->hdrTarget->width, 1.0f / renderer->hdrTarget->height, 0, 0);

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTextureSamplerBinding textureBindings[MAX_COLOR_ATTACHMENTS + 1];
		for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
		{
			textureBindings[i].texture = renderer->gbuffer->colorAttachments[i];
			textureBindings[i].sampler = renderer->defaultSampler;
		}
		textureBindings[renderer->gbuffer->numColorAttachments].texture = renderer->gbuffer->depthAttachment;
		textureBindings[renderer->gbuffer->numColorAttachments].sampler = renderer->defaultSampler;

		SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, renderer->gbuffer->numColorAttachments + 1);

		SDL_DrawGPUIndexedPrimitives(renderPass, renderer->cubeIndexBuffer->numIndices, renderer->pointLights.size, 0, 0, 0);
	}
}
