


static void AutoExposure(Renderer* renderer)
{
	GPU_SCOPE("AutoExposure");
	
	if (!renderer->luminanceReadbackFence)
	{
		SDL_GenerateMipmapsForGPUTexture(cmdBuffer, renderer->hdrTarget->colorAttachments[0]);

		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

		int mip = GetNumMipsForTexture(renderer->width, renderer->height) - 2;
		ivec2 mipSize = GetMipSize(renderer->width, renderer->height, mip);

		SDL_GPUTextureRegion src = {};
		src.texture = renderer->hdrTarget->colorAttachments[0];  /**< The texture used in the copy operation. */
		src.mip_level = mip;         /**< The mip level index to transfer. */
		src.layer = 0;             /**< The layer index to transfer. */
		src.x = 0;                 /**< The left offset of the region. */
		src.y = 0;                 /**< The top offset of the region. */
		src.z = 0;                 /**< The front offset of the region. */
		src.w = 1;                 /**< The width of the region. */
		src.h = 1;                 /**< The height of the region. */
		src.d = 1;                 /**< The depth of the region. */

		SDL_GPUTextureTransferInfo dst = {};
		dst.transfer_buffer = renderer->luminanceReadbackBuffer;  /**< The transfer buffer used in the transfer operation. */
		dst.offset = 0;                           /**< The starting byte of the image data in the transfer buffer. */
		dst.pixels_per_row = mipSize.x;                   /**< The number of pixels from one row to the next. */
		dst.rows_per_layer = mipSize.y;                   /**< The number of rows from one layer/depth-slice to the next. */

		SDL_DownloadFromGPUTexture(copyPass, &src, &dst);

		SDL_EndGPUCopyPass(copyPass);

		app->acquireFence = true;
		app->fenceTarget = &renderer->luminanceReadbackFence;
	}
	else
	{
		if (SDL_QueryGPUFence(device, renderer->luminanceReadbackFence))
		{
			int mip = GetNumMipsForTexture(renderer->width, renderer->height) - 2;
			ivec2 mipSize = GetMipSize(renderer->width, renderer->height, mip);

			uint32_t* mappedBuffer = (uint32_t*)SDL_MapGPUTransferBuffer(device, renderer->luminanceReadbackBuffer, true);

			vec3 rgb = vec3(0);
			for (int i = 0; i < mipSize.x* mipSize.y; i++)
				rgb += DecodeRG11B10(mappedBuffer[i]);
			rgb /= (float)(mipSize.x * mipSize.y);

			float luminance = dot(rgb, vec3(0.3f, 0.59f, 0.11f));
			float minExposure = 0.5f;
			float maxExposure = 10;
			renderer->targetExposure = clamp(sqrtf(0.18f / luminance * 2), minExposure, maxExposure); // luminance of 0.18 corresponds to middle gray

			SDL_UnmapGPUTransferBuffer(device, renderer->luminanceReadbackBuffer);

			SDL_ReleaseGPUFence(device, renderer->luminanceReadbackFence);
			renderer->luminanceReadbackFence = nullptr;
		}
	}

	float adaptionSpeed = renderer->currentExposure > renderer->targetExposure ? 1 : 0.5f;
	renderer->currentExposure = mix(renderer->currentExposure, renderer->targetExposure, adaptionSpeed * deltaTime);
}
