

static void Bloom(Renderer* renderer, SDL_GPUTexture* input)
{
	{
		GPU_SCOPE("Bloom Downsample");

		int width = renderer->width / 2;
		int height = renderer->height / 2;

		for (int i = 0; i < renderer->bloomStepCount; i++)
		{
			SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
			bufferBinding.texture = renderer->bloomDownsampleBuffer;
			bufferBinding.mip_level = i;
			bufferBinding.layer = 0;
			bufferBinding.cycle = false;

			SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

			SDL_BindGPUComputePipeline(computePass, renderer->bloomDownsampleShader->compute);

			vec4 params = vec4((float)i, 0, 0, 0);
			SDL_PushGPUComputeUniformData(cmdBuffer, 0, &params, sizeof(params));

			SDL_GPUTextureSamplerBinding bindings[2];
			bindings[0].texture = input;
			bindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];
			bindings[1].texture = renderer->exposureBuffer;
			bindings[1].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
			SDL_BindGPUComputeSamplers(computePass, 0, bindings, 2);

			SDL_DispatchGPUCompute(computePass, (width + 31) / 32, (height + 31) / 32, 1);

			SDL_EndGPUComputePass(computePass);

			input = renderer->bloomDownsampleBuffer;

			width = max(width >> 1, 1);
			height = max(height >> 1, 1);
		}
	}

	{
		GPU_SCOPE("Bloom Upsample");

		for (int i = renderer->bloomStepCount - 2; i >= 0; i--)
		{
			SDL_GPUTexture* input1 = renderer->bloomDownsampleBuffer;

			SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
			bufferBinding.texture = renderer->bloomUpsampleBuffer;
			bufferBinding.mip_level = i;
			bufferBinding.layer = 0;
			bufferBinding.cycle = false;

			SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

			SDL_BindGPUComputePipeline(computePass, renderer->bloomUpsampleShader->compute);

			vec4 params = vec4((float)i, 0, 0, 0);
			SDL_PushGPUComputeUniformData(cmdBuffer, 0, &params, sizeof(params));

			SDL_GPUTextureSamplerBinding bindings[2];
			bindings[0].texture = input;
			bindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];
			bindings[1].texture = input1;
			bindings[1].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];
			SDL_BindGPUComputeSamplers(computePass, 0, bindings, 2);

			int width, height;
			GetMipSize(renderer->width / 2, renderer->height / 2, i, &width, &height);

			SDL_DispatchGPUCompute(computePass, (width + 31) / 32, (height + 31) / 32, 1);

			SDL_EndGPUComputePass(computePass);

			input = renderer->bloomUpsampleBuffer;
		}
	}
}
