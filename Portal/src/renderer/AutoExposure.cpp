

static void AutoExposure(Renderer* renderer, SDL_GPUTexture* input)
{
	{
		GPU_SCOPE("HDR Downsample");

		int width = 64;
		int height = 64;

		for (int i = 0; i < 7; i++)
		{
			SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
			bufferBinding.texture = renderer->luminanceDownsampleBuffer;
			bufferBinding.mip_level = i;
			bufferBinding.layer = 0;
			bufferBinding.cycle = false;

			SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

			Shader* shader = i == 0 ? renderer->hdrToLuminanceShader : renderer->luminanceDownsampleShader;

			SDL_BindGPUComputePipeline(computePass, shader->compute);

			if (i > 0)
			{
				vec4 params = vec4((float)i, 0, 0, 0);
				SDL_PushGPUComputeUniformData(cmdBuffer, 0, &params, sizeof(params));
			}

			SDL_GPUTextureSamplerBinding bindings[1];
			bindings[0].texture = input;
			bindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];
			SDL_BindGPUComputeSamplers(computePass, 0, bindings, 1);

			SDL_DispatchGPUCompute(computePass, (width + 7) / 8, (height + 7) / 8, 1);

			SDL_EndGPUComputePass(computePass);

			SDL_assert(width && height);
			width >>= 1;
			height >>= 1;

			input = renderer->luminanceDownsampleBuffer;
		}
	}

	{
		GPU_TIMER("Auto exposure");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->exposureBuffer;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->autoExposureShader->compute);

		SDL_GPUTextureSamplerBinding inputBinding = {};
		inputBinding.texture = renderer->luminanceDownsampleBuffer;
		inputBinding.sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
		SDL_BindGPUComputeSamplers(computePass, 0, &inputBinding, 1);

		vec4 params = vec4(deltaTime, 0, 0, 0);
		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &params, sizeof(params));

		SDL_DispatchGPUCompute(computePass, 1, 1, 1);

		SDL_EndGPUComputePass(computePass);
	}
}
