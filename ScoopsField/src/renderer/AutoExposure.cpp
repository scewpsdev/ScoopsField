


static void AutoExposure(Renderer* renderer, RenderTarget* input, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("AutoExposure");

	if (!renderer->luminanceReadbackFence)
	{
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		SDL_GPUTextureRegion src = {};
		src.texture = input->colorAttachments[0];
		src.mip_level = 0;
		src.layer = 0;
		src.x = 0;
		src.y = 0;
		src.z = 0;
		src.w = 1;
		src.h = 1;
		src.d = 1;

		SDL_GPUTextureTransferInfo dst = {};
		dst.transfer_buffer = renderer->luminanceReadbackBuffer;
		dst.offset = 0;
		dst.pixels_per_row = input->width;
		dst.rows_per_layer = input->height;

		SDL_DownloadFromGPUTexture(copyPass, &src, &dst);

		SDL_EndGPUCopyPass(copyPass);

		app->acquireFence = true;
		app->fenceTarget = &renderer->luminanceReadbackFence;
	}
	else
	{
		if (SDL_QueryGPUFence(device, renderer->luminanceReadbackFence))
		{
			uint32_t* mappedBuffer = (uint32_t*)SDL_MapGPUTransferBuffer(device, renderer->luminanceReadbackBuffer, true);

			vec3 rgb = vec3(0);
			for (int i = 0; i < input->width* input->height; i++)
				rgb += DecodeRG11B10(mappedBuffer[i]);
			rgb /= (float)(input->width * input->height);

			float luminance = dot(rgb, vec3(0.3f, 0.59f, 0.11f));
			float minExposure = 0.1f;
			float maxExposure = 100;
			renderer->targetExposure = clamp(powf(0.18f / luminance, 0.5f), minExposure, maxExposure); // luminance of 0.18 corresponds to middle gray

			SDL_UnmapGPUTransferBuffer(device, renderer->luminanceReadbackBuffer);

			SDL_ReleaseGPUFence(device, renderer->luminanceReadbackFence);
			renderer->luminanceReadbackFence = nullptr;
		}
	}

	float adaptionSpeed = renderer->currentExposure > renderer->targetExposure ? 1 : 0.5f;
	renderer->currentExposure = mix(renderer->currentExposure, renderer->targetExposure, adaptionSpeed * deltaTime);
}
