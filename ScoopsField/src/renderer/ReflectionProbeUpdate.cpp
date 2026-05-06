


static void UpdateReflectionProbes(Renderer* renderer, vec3 sunDirection, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (renderer->reflectionProbeUpdates.size > 0)
	{
		GPU_SCOPE("Reflection Probe Update");

		ReflectionProbe* probe = renderer->reflectionProbeUpdates[0];
		renderer->reflectionProbeUpdates.removeAt(0);

		quat cubemapRotations[6];
		cubemapRotations[SDL_GPU_CUBEMAPFACE_POSITIVEX] = quat::FromAxisAngle(vec3::Up, -0.5f * PI);
		cubemapRotations[SDL_GPU_CUBEMAPFACE_NEGATIVEX] = quat::FromAxisAngle(vec3::Up, 0.5f * PI);
		cubemapRotations[SDL_GPU_CUBEMAPFACE_POSITIVEY] = quat::FromAxisAngle(vec3::Right, 0.5f * PI);
		cubemapRotations[SDL_GPU_CUBEMAPFACE_NEGATIVEY] = quat::FromAxisAngle(vec3::Right, -0.5f * PI);
		cubemapRotations[SDL_GPU_CUBEMAPFACE_POSITIVEZ] = quat::FromAxisAngle(vec3::Up, PI);
		cubemapRotations[SDL_GPU_CUBEMAPFACE_NEGATIVEZ] = quat::Identity;

		mat4 projection = mat4::Perspective(PI * 0.5f, 1, 0.1f);

		mat4 views[6];
		mat4 pvs[6];

		{
			GPU_SCOPE("Geometry");

			for (int face = 0; face < 6; face++)
			{
				views[face] = mat4::Rotate(cubemapRotations[face].conjugated()) * mat4::Translate(-probe->position);
				pvs[face] = projection * views[face];

				SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->cubemapGbuffers[face], 0, cmdBuffer);

				SDL_BindGPUGraphicsPipeline(renderPass, renderer->geometryPipeline->pipeline);

				for (int i = 0; i < renderer->meshes.size; i++)
				{
					Mesh* mesh = renderer->meshes[i].mesh;
					Material* material = renderer->meshes[i].material;
					const mat4& transform = renderer->meshes[i].transform;
					SubmitMesh(renderer, mesh, material, nullptr, transform, views[face], pvs[face], renderPass, cmdBuffer);
				}

				SDL_BindGPUGraphicsPipeline(renderPass, renderer->animatedPipeline->pipeline);

				for (int i = 0; i < renderer->animatedMeshes.size; i++)
				{
					Mesh* mesh = renderer->animatedMeshes[i].mesh;
					Material* material = renderer->animatedMeshes[i].material;
					SkeletonState* skeleton = renderer->animatedMeshes[i].skeleton;
					const mat4& transform = renderer->animatedMeshes[i].transform;
					SubmitMesh(renderer, mesh, material, skeleton, transform, views[face], pvs[face], renderPass, cmdBuffer);
				}

				SDL_EndGPURenderPass(renderPass);
			}
		}

		{
			GPU_SCOPE("Deferred");

			for (int i = 0; i < 6; i++)
			{
				mat4 pvInv = pvs[i].inverted();
				mat4 projectionInv = projection.inverted();
				mat4 viewInv = views[i].inverted();

				SDL_GPURenderPass* renderPass = BindRenderTarget(probe->cubemap, i, cmdBuffer);

				SDL_BindGPUGraphicsPipeline(renderPass, renderer->deferredDiffusePipeline->pipeline);

				struct UniformData
				{
					mat4 projectionViewInv;
					mat4 projectionInv;
					mat4 viewInv;
					vec4 params;
					vec4 params2;
				};

				UniformData uniforms = {};
				uniforms.projectionViewInv = pvInv;
				uniforms.projectionInv = projectionInv;
				uniforms.viewInv = viewInv;
				uniforms.params = vec4(sunDirection, 0);
				uniforms.params2 = vec4(probe->position, 0);

				SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

				SDL_GPUTexture* textures[6];
				textures[0] = renderer->cubemapGbuffers[i]->colorAttachments[0];
				textures[1] = renderer->cubemapGbuffers[i]->colorAttachments[1];
				textures[2] = renderer->cubemapGbuffers[i]->colorAttachments[2];
				textures[3] = renderer->cubemapGbuffers[i]->depthAttachment;
				textures[4] = renderer->sunColorBuffer;
				textures[5] = renderer->skyCubemap->colorAttachments[0];

				SDL_GPUSampler* samplers[6];
				samplers[0] = renderer->defaultSampler;
				samplers[1] = renderer->defaultSampler;
				samplers[2] = renderer->defaultSampler;
				samplers[3] = renderer->defaultSampler;
				samplers[4] = renderer->clampedSampler;
				samplers[5] = renderer->linearSampler;

				RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 6, textures, samplers, cmdBuffer);

				SDL_EndGPURenderPass(renderPass);
			}
		}

		SDL_GenerateMipmapsForGPUTexture(cmdBuffer, probe->cubemap->colorAttachments[0]);

		{
			GPU_TIMER("Convolution");

			SDL_GPUStorageBufferReadWriteBinding bufferBinding = {};
			bufferBinding.buffer = probe->irradiance;
			bufferBinding.cycle = false;

			SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, nullptr, 0, &bufferBinding, 1);

			SDL_BindGPUComputePipeline(computePass, renderer->shConvoluteShader->compute);

			SDL_GPUTextureSamplerBinding bindings[1];
			bindings[0].texture = probe->cubemap->colorAttachments[0];
			bindings[0].sampler = renderer->defaultSampler;
			SDL_BindGPUComputeSamplers(computePass, 0, bindings, 1);

			SDL_DispatchGPUCompute(computePass, 1, 1, 1);

			SDL_EndGPUComputePass(computePass);
		}
	}
}
