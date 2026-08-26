


static void UpdateReflectionProbes(Renderer* renderer, vec3 sunDirection, vec3 cameraPosition, SDL_GPUCommandBuffer* cmdBuffer)
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
		cubemapRotations[SDL_GPU_CUBEMAPFACE_POSITIVEZ] = quat::Identity;
		cubemapRotations[SDL_GPU_CUBEMAPFACE_NEGATIVEZ] = quat::FromAxisAngle(vec3::Up, PI);

		mat4 projection = mat4::Perspective(PI * 0.5f, 1, 0.1f);

		mat4 views[6];
		mat4 pvs[6];

		{
			GPU_SCOPE("Geometry");

			for (int face = 0; face < 6; face++)
			{
				views[face] = mat4::Rotate(cubemapRotations[face].conjugated()) * mat4::Translate(-probe->position);
				pvs[face] = projection * views[face];

				vec4 frustumPlanes[6];
				GetFrustumPlanes(pvs[face], frustumPlanes);

				SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->cubemapGbuffers[face], 0, cmdBuffer);

				SDL_BindGPUGraphicsPipeline(renderPass, renderer->geometryPipeline->pipeline);

				for (int i = 0; i < renderer->meshes.size; i++)
				{
					MeshDrawData* mesh = &renderer->meshes[i];
					bool renderToReflection = mesh->flags & MESH_DRAW_FLAG_RENDER_TO_REFLECTION;
					if (renderToReflection && FrustumCulling(mesh->boundingSphere, mesh->transform, frustumPlanes))
						SubmitMesh(renderer, mesh, projection, views[face], pvs[face], cameraPosition, sunDirection, true, vec4(0), renderPass, cmdBuffer);
				}

				SDL_BindGPUGraphicsPipeline(renderPass, renderer->animatedPipeline->pipeline);

				for (int i = 0; i < renderer->animatedMeshes.size; i++)
				{
					MeshDrawData* mesh = &renderer->animatedMeshes[i];
					bool renderToReflection = mesh->flags & MESH_DRAW_FLAG_RENDER_TO_REFLECTION;
					if (renderToReflection && FrustumCulling(mesh->boundingSphere, mesh->transform, frustumPlanes))
						SubmitMesh(renderer, mesh, projection, views[face], pvs[face], cameraPosition, sunDirection, true, vec4(0), renderPass, cmdBuffer);
				}

				SDL_EndGPURenderPass(renderPass);
			}
		}

		mat4 shadowPV;

		{
			GPU_SCOPE("Shadow Map");

			mat4 projection, view;
			CalculateShadowMatricesForAABB(probe->position, probe->size, sunDirection, &projection, &view);

			shadowPV = projection * view;

			vec4 frustumPlanes[6];
			GetFrustumPlanes(shadowPV, frustumPlanes);

			SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->reflectionProbeShadowMap, 0, cmdBuffer);

			RenderShadowMapGeometry(renderer, renderPass, projection, view, shadowPV, cameraPosition, sunDirection, frustumPlanes, cmdBuffer);

			SDL_EndGPURenderPass(renderPass);
		}

		{
			GPU_SCOPE("Deferred");

			vec4 pointLightPosition[4];
			vec4 pointLightColor[4];
			int numPointLights;

			GetClosestPointLights(renderer, probe->position, pointLightPosition, pointLightColor, &numPointLights);

			ReflectionProbe* reflectionProbes[4];
			vec4 reflectionProbePosition[4];
			vec4 reflectionProbeSize[4];
			SDL_GPUBuffer* irradiances[5];
			int numReflectionProbes;

			GetClosestReflectionProbes(renderer, probe->position, probe, reflectionProbes, &numReflectionProbes);

			irradiances[0] = probe->irradiance;
			for (int i = 0; i < numReflectionProbes; i++)
			{
				reflectionProbePosition[i] = vec4(reflectionProbes[i]->position, 0);
				reflectionProbeSize[i] = vec4(reflectionProbes[i]->size, 0);
				irradiances[1 + i] = reflectionProbes[i]->irradiance;
			}


			for (int i = 0; i < 6; i++)
			{
				mat4 pvInv = pvs[i].inverted();
				mat4 projectionInv = projection.inverted();
				mat4 viewInv = views[i].inverted();

				SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->reflectionProbeTarget, i, cmdBuffer);

				SDL_BindGPUGraphicsPipeline(renderPass, renderer->deferredDiffusePipeline->pipeline);

				struct UniformData
				{
					mat4 projectionViewInv;
					mat4 projectionInv;
					mat4 viewInv;
					mat4 toLightSpace;
					vec4 params;
					vec4 params2;
					vec4 params3;
					vec4 params4;

					vec4 pointLightPosition[4];
					vec4 pointLightColor[4];
					vec4 reflectionProbePosition[4];
					vec4 reflectionProbeSize[4];
					vec4 params5;
				};

				UniformData uniforms = {};
				uniforms.projectionViewInv = pvInv;
				uniforms.projectionInv = projectionInv;
				uniforms.viewInv = viewInv;
				uniforms.toLightSpace = shadowPV;
				uniforms.params = vec4(sunDirection, 0);
				uniforms.params2 = vec4(probe->position, 0);
				uniforms.params3 = vec4(probe->position, 0);
				uniforms.params4 = vec4(probe->size, 0);

				SDL_memcpy(uniforms.pointLightPosition, pointLightPosition, sizeof(pointLightPosition));
				SDL_memcpy(uniforms.pointLightColor, pointLightColor, sizeof(pointLightColor));
				SDL_memcpy(uniforms.reflectionProbePosition, reflectionProbePosition, sizeof(reflectionProbePosition));
				SDL_memcpy(uniforms.reflectionProbeSize, reflectionProbeSize, sizeof(reflectionProbeSize));
				uniforms.params5.x = (float)numPointLights;
				uniforms.params5.y = (float)numReflectionProbes;

				SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

				SDL_BindGPUFragmentStorageBuffers(renderPass, 0, irradiances, 1 + numReflectionProbes);

				SDL_GPUTexture* textures[7];
				textures[0] = renderer->cubemapGbuffers[i]->colorAttachments[0];
				textures[1] = renderer->cubemapGbuffers[i]->colorAttachments[1];
				textures[2] = renderer->cubemapGbuffers[i]->colorAttachments[2];
				textures[3] = renderer->cubemapGbuffers[i]->depthAttachment;
				textures[4] = renderer->sunColorBuffer;
				textures[5] = renderer->reflectionProbeShadowMap->depthAttachment;
				textures[6] = renderer->skybox->handle;

				SDL_GPUSampler* samplers[7];
				samplers[0] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
				samplers[1] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
				samplers[2] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
				samplers[3] = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
				samplers[4] = renderer->samplers[TEXTURE_SAMPLER_CLAMPED];
				samplers[5] = renderer->samplers[TEXTURE_SAMPLER_SHADOW_LINEAR_CLAMPED];
				samplers[6] = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED_VERTICAL];

				RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 7, textures, samplers, cmdBuffer);

				SDL_EndGPURenderPass(renderPass);
			}
		}

		SDL_GenerateMipmapsForGPUTexture(cmdBuffer, renderer->reflectionProbeTarget->colorAttachments[0]);

		{
			GPU_TIMER("SH Convolution");

			SDL_GPUStorageBufferReadWriteBinding bufferBinding = {};
			bufferBinding.buffer = probe->irradiance;
			bufferBinding.cycle = false;

			SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, nullptr, 0, &bufferBinding, 1);

			SDL_BindGPUComputePipeline(computePass, renderer->shConvoluteShader->compute);

			SDL_GPUTextureSamplerBinding bindings[1];
			bindings[0].texture = renderer->reflectionProbeTarget->colorAttachments[0];
			bindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_DEFAULT];
			SDL_BindGPUComputeSamplers(computePass, 0, bindings, 1);

			SDL_DispatchGPUCompute(computePass, 1, 1, 1);

			SDL_EndGPUComputePass(computePass);
		}

		{
			GPU_TIMER("Specular Convolution");

			int width = 32, height = 32;
			int numSteps = GetNumMipsForTexture(width, height);
			int maxLod = 5;

			for (int i = 0; i <= maxLod; i++)
			{
				SDL_GPUStorageTextureReadWriteBinding bufferBindings[6];

				for (int face = 0; face < 6; face++)
				{
					bufferBindings[face].texture = probe->specular;
					bufferBindings[face].mip_level = i;
					bufferBindings[face].layer = face;
					bufferBindings[face].cycle = false;
				}

				SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, bufferBindings, 6, nullptr, 0);

				SDL_BindGPUComputePipeline(computePass, renderer->specularConvoluteShader->compute);

				float roughness = i / (float)maxLod;
				vec4 params = vec4(roughness, 0, 0, 0);
				SDL_PushGPUComputeUniformData(cmdBuffer, 0, &params, sizeof(params));

				SDL_GPUTextureSamplerBinding bindings[1];
				bindings[0].texture = renderer->reflectionProbeTarget->colorAttachments[0];
				bindings[0].sampler = renderer->samplers[TEXTURE_SAMPLER_LINEAR];
				SDL_BindGPUComputeSamplers(computePass, 0, bindings, 1);

				SDL_DispatchGPUCompute(computePass, max(1, width / 16), max(1, height / 16), 6);

				SDL_EndGPUComputePass(computePass);

				width = max(width >> 1, 1);
				height = max(height >> 1, 1);
			}
		}
	}
}
