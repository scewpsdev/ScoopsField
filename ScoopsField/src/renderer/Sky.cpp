



static void RenderSky(Renderer* renderer, vec3 cameraPosition, mat4 projectionInv, mat4 viewInv, vec3 sunDirection, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("Sky");

	// transmittance lut
	{
		GPU_TIMER("transmittance lut");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->skyTransmittanceLUT;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->skyTransmittaceLUTShader->compute);

		SDL_GPUTextureSamplerBinding bindings[2];
		bindings[0].texture = renderer->emptyTexture;
		bindings[0].sampler = renderer->defaultSampler;
		bindings[1].texture = renderer->emptyTexture;
		bindings[1].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 2);

		struct UniformData
		{
			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 256 / 32, 64 / 32, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// multiscatter lut
	{
		GPU_TIMER("multiscatter lut");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->skyMultiScatterLUT;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->skyMultiScatterLUTShader->compute);

		SDL_GPUTextureSamplerBinding bindings[2];
		bindings[0].texture = renderer->skyTransmittanceLUT;
		bindings[0].sampler = renderer->linearClampedSampler;
		bindings[1].texture = renderer->emptyTexture;
		bindings[1].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 2);

		struct UniformData
		{
			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 32 / 32, 32 / 32, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// sky view lut
	{
		GPU_TIMER("sky view lut");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->skyViewLUT;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->skyViewLUTShader->compute);

		SDL_GPUTextureSamplerBinding bindings[3];
		bindings[0].texture = renderer->skyTransmittanceLUT;
		bindings[0].sampler = renderer->linearClampedSampler;
		bindings[1].texture = renderer->skyMultiScatterLUT;
		bindings[1].sampler = renderer->linearClampedSampler;
		bindings[2].texture = renderer->blueNoise->handle;
		bindings[2].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 3);

		struct UniformData
		{
			vec4 params;
			vec4 params2;

			vec4 weatherData;
		};
		UniformData uniforms = {};
		uniforms.params = vec4(sunDirection, 0);
		uniforms.params2 = vec4(cameraPosition, 0);
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 256 / 32, 128 / 32, 1);

		SDL_EndGPUComputePass(computePass);
	}

	/*
	// cloud noise
	{
		GPU_TIMER("cloud noise");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->cloudNoise;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->cloudNoiseShader->compute);

		SDL_DispatchGPUCompute(computePass, 128 / 8, 128 / 8, 128 / 8);

		SDL_EndGPUComputePass(computePass);
	}

	// cloud noise detail
	{
		GPU_TIMER("cloud noise detail");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->cloudNoiseDetail;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->cloudNoiseDetailShader->compute);

		SDL_DispatchGPUCompute(computePass, 32 / 8, 32 / 8, 32 / 8);

		SDL_EndGPUComputePass(computePass);
	}
	*/

	// sun color
	{
		GPU_TIMER("sun color");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->sunColorBuffer;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->sunColorShader->compute);

		SDL_GPUTextureSamplerBinding bindings[7];
		bindings[0].texture = renderer->cloudLowFrequency->handle;
		bindings[0].sampler = renderer->linearSampler;
		bindings[1].texture = renderer->emptyTexture;
		bindings[1].sampler = renderer->defaultSampler;
		bindings[2].texture = renderer->emptyTexture;
		bindings[2].sampler = renderer->defaultSampler;
		bindings[3].texture = renderer->skyTransmittanceLUT;
		bindings[3].sampler = renderer->linearClampedSampler;
		bindings[4].texture = renderer->emptyTexture;
		bindings[4].sampler = renderer->defaultSampler;
		bindings[5].texture = renderer->cloudNoise;
		bindings[5].sampler = renderer->linearSampler;
		bindings[6].texture = renderer->cloudNoiseDetail;
		bindings[6].sampler = renderer->linearSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 7);

		struct UniformData
		{
			vec4 params;
			vec4 params2;

			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(sunDirection, gameTime);
		uniforms.params2 = vec4(cameraPosition, 0);
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 1, 1, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// sky
	{
		GPU_TIMER("clouds");

		RenderTarget* rt = app->frameIdx % 2 == 0 ? renderer->skyTarget : renderer->skyTarget2;
		RenderTarget* rt2 = app->frameIdx % 2 == 0 ? renderer->skyTarget2 : renderer->skyTarget;

		SDL_GPURenderPass* renderPass = BindRenderTarget(rt, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->skyPipeline->pipeline);

		struct UniformData
		{
			vec4 params;
			vec4 params2;
			mat4 projectionInv;
			mat4 viewInv;
			mat4 lastProjection;
			mat4 lastView;

			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(sunDirection, gameTime);
		uniforms.params2 = vec4((float)app->frameIdx, 0, 0, 0);
		uniforms.projectionInv = projectionInv;
		uniforms.viewInv = viewInv;
		uniforms.lastProjection = renderer->lastProjection;
		uniforms.lastView = renderer->lastView;
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[11];
		gbufferTextures[0] = renderer->gbuffer->depthAttachment;
		gbufferTextures[1] = renderer->cloudCoverage->handle;
		gbufferTextures[2] = renderer->cloudLowFrequency->handle;
		gbufferTextures[3] = renderer->cloudHighFrequency->handle;
		gbufferTextures[4] = renderer->blueNoise->handle;
		gbufferTextures[5] = rt2->colorAttachments[0];
		gbufferTextures[6] = renderer->skyTransmittanceLUT;
		gbufferTextures[7] = renderer->skyMultiScatterLUT;
		gbufferTextures[8] = renderer->skyViewLUT;
		gbufferTextures[9] = renderer->cloudNoise;
		gbufferTextures[10] = renderer->cloudNoiseDetail;

		SDL_GPUSampler* samplers[11];
		samplers[0] = renderer->defaultSampler;
		samplers[1] = renderer->linearSampler;
		samplers[2] = renderer->linearSampler;
		samplers[3] = renderer->linearSampler;
		samplers[4] = renderer->defaultSampler;
		samplers[5] = renderer->linearClampedSampler;
		samplers[6] = renderer->linearClampedSampler;
		samplers[7] = renderer->linearClampedSampler;
		samplers[8] = renderer->linearClampedVSampler;
		samplers[9] = renderer->linearSampler;
		samplers[10] = renderer->linearSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 11, gbufferTextures, samplers, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	// sky cubemap
	{
		GPU_TIMER("cubemap");

		mat4 cubemapViewsInv[6];
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_POSITIVEX] = mat4::Rotate(vec3::Up, -0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_NEGATIVEX] = mat4::Rotate(vec3::Up, 0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_POSITIVEY] = mat4::Rotate(vec3::Right, 0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_NEGATIVEY] = mat4::Rotate(vec3::Right, -0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_POSITIVEZ] = mat4::Rotate(vec3::Up, PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_NEGATIVEZ] = mat4::Identity;

		for (int i = 0; i < 6; i++)
		{
			SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->skyCubemap, i, cmdBuffer);

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->skyCubePipeline->pipeline);

			struct UniformData
			{
				vec4 params;
				vec4 params2;
				mat4 projectionInv;
				mat4 viewInv;

				vec4 weatherData;
			};

			UniformData uniforms = {};
			uniforms.params = vec4(sunDirection, gameTime);
			uniforms.params2 = vec4(cameraPosition, (float)app->frameIdx);
			uniforms.projectionInv = mat4::Perspective(0.5f * PI, 1, 0.1f);
			uniforms.viewInv = cubemapViewsInv[i];
			uniforms.weatherData = renderer->weather.getData();

			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTexture* gbufferTextures[9];
			gbufferTextures[0] = renderer->cloudCoverage->handle;
			gbufferTextures[1] = renderer->cloudLowFrequency->handle;
			gbufferTextures[2] = renderer->cloudHighFrequency->handle;
			gbufferTextures[3] = renderer->blueNoise->handle;
			gbufferTextures[4] = renderer->skyTransmittanceLUT;
			gbufferTextures[5] = renderer->skyMultiScatterLUT;
			gbufferTextures[6] = renderer->skyViewLUT;
			gbufferTextures[7] = renderer->cloudNoise;
			gbufferTextures[8] = renderer->cloudNoiseDetail;

			SDL_GPUSampler* samplers[9];
			samplers[0] = renderer->linearSampler;
			samplers[1] = renderer->linearSampler;
			samplers[2] = renderer->linearSampler;
			samplers[3] = renderer->defaultSampler;
			samplers[4] = renderer->linearClampedSampler;
			samplers[5] = renderer->linearClampedSampler;
			samplers[6] = renderer->linearClampedVSampler;
			samplers[7] = renderer->linearSampler;
			samplers[8] = renderer->linearSampler;

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 9, gbufferTextures, samplers, cmdBuffer);

			SDL_EndGPURenderPass(renderPass);
		}

		SDL_GenerateMipmapsForGPUTexture(cmdBuffer, renderer->skyCubemap->colorAttachments[0]);
	}
}