


static void Downsample(Renderer* renderer, SDL_GPUTexture* input, RenderTarget* target, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPURenderPass* renderPass = BindRenderTarget(target, 0, cmdBuffer);

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->bloomDownsamplePipeline->pipeline);

	RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 1, &input, &renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED], cmdBuffer);

	SDL_EndGPURenderPass(renderPass);
}

static void Upsample(Renderer* renderer, SDL_GPUTexture* input0, SDL_GPUTexture* input1, RenderTarget* target, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPURenderPass* renderPass = BindRenderTarget(target, 0, cmdBuffer);

	SDL_BindGPUGraphicsPipeline(renderPass, renderer->bloomUpsamplePipeline->pipeline);

	SDL_GPUTexture* textures[2];
	textures[0] = input0;
	textures[1] = input1;

	SDL_GPUSampler* samplers[2];
	samplers[0] = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];
	samplers[1] = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];

	RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 2, textures, samplers, cmdBuffer);

	SDL_EndGPURenderPass(renderPass);
}

static void Bloom(Renderer* renderer, SDL_GPUTexture* input, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("Bloom");

	{
		GPU_SCOPE("Downsample");

		for (int i = 0; i < renderer->bloomStepCount; i++)
		{
			RenderTarget* target = renderer->bloomDownsampleTargets[i];
			Downsample(renderer, input, target, cmdBuffer);
			input = target->colorAttachments[0];
		}
	}

	{
		GPU_SCOPE("Upsample");

		for (int i = renderer->bloomStepCount - 2; i >= 0; i--)
		{
			SDL_GPUTexture* input1 = renderer->bloomDownsampleTargets[i]->colorAttachments[0];
			RenderTarget* target = renderer->bloomUpsampleTargets[i];
			Upsample(renderer, input, input1, target, cmdBuffer);
			input = target->colorAttachments[0];
		}
	}
}
