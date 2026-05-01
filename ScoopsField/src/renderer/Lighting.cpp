


static void Lighting(Renderer* renderer, vec3 cameraPosition, float near, mat4 projection, mat4 view, mat4 pv, mat4 projectionInv, mat4 viewInv, mat4 pvInv, vec4 frustumPlanes[6], vec3 sunDirection, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("Lighting");

	// environment light
	{
		GPU_TIMER("environment");

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
		GPU_TIMER("sun");

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
		};

		UniformData uniforms = {};
		uniforms.lightDirection = vec4(lightDirection, gameTime);
		uniforms.lightColor = vec4(lightColor, 0);
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
		samplers[0] = renderer->defaultSampler;
		samplers[1] = renderer->defaultSampler;
		samplers[2] = renderer->defaultSampler;
		samplers[3] = renderer->defaultSampler;
		samplers[4] = renderer->defaultSampler;
		samplers[5] = renderer->clampedSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 6, gbufferTextures, samplers, cmdBuffer);
	}

	// point lights
	{
		GPU_TIMER("point lights");

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
