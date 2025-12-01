#include "Texture.h"

#include "Application.h"

#include "TextureFormat.h"

#include "io/Image.h"
#include "io/BinaryReader.h"

#include <SDL3/SDL.h>


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


static void LoadKTX11Header(BinaryReader& reader, TextureInfo* info)
{
	//reader.AssertASCII("«KTX");
	StringView identifier = reader.ReadASCII(12);
	SDL_assert(strncmp(identifier.buffer, "«KTX", 4) == 0);

	uint32_t endianness = reader.ReadUInt32();
	bool littleEndian = endianness == 0x04030201;

	uint32_t type = reader.ReadUInt32();
	uint32_t typeSize = reader.ReadUInt32();
	uint32_t format = reader.ReadUInt32();
	uint32_t internalFormat = reader.ReadUInt32();
	uint32_t baseInternalFormat = reader.ReadUInt32();
	uint32_t width = reader.ReadUInt32();
	uint32_t height = reader.ReadUInt32();
	uint32_t depth = reader.ReadUInt32();
	uint32_t layerCount = reader.ReadUInt32();
	uint32_t faceCount = reader.ReadUInt32();
	uint32_t levelCount = reader.ReadUInt32();
	uint32_t metadataSize = reader.ReadUInt32();

	info->format = TranslateTextureFormat(internalFormat, baseInternalFormat); // vkTextureFormatTranslation[internalFormat];
	info->width = (int)width;
	info->height = (int)height;
	info->depth = depth != 0 ? (int)depth : 1;
	info->numMips = (int)levelCount;
	info->numLayers = layerCount != 0 ? (int)layerCount : 1;
	info->numFaces = (int)faceCount;

	reader.Skip(metadataSize);
}

Texture* LoadTexture(const char* path, SDL_GPUCommandBuffer* cmdBuffer)
{
	size_t fileSize;
	void* data = SDL_LoadFile(path, &fileSize);

	BinaryReader reader((uint8_t*)data, (int)fileSize);

	TextureInfo info = {};
	LoadKTX11Header(reader, &info);

	uint32_t imageSize = reader.ReadUInt32();
	uint8_t* textureData = reader.CurrentPtr();

	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.format = info.format;
	textureInfo.width = info.width;
	textureInfo.height = info.height;
	textureInfo.layer_count_or_depth = info.depth > 1 ? info.depth : info.numLayers;
	textureInfo.num_levels = info.numMips;
	textureInfo.type = info.depth > 1 ? SDL_GPU_TEXTURETYPE_3D : info.numLayers > 1 && info.numFaces == 1 ? SDL_GPU_TEXTURETYPE_2D_ARRAY : info.numFaces > 1 ? SDL_GPU_TEXTURETYPE_CUBE : info.numLayers > 1 && info.numFaces > 1 ? SDL_GPU_TEXTURETYPE_CUBE_ARRAY : SDL_GPU_TEXTURETYPE_2D;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

	SDL_GPUTexture* handle = SDL_CreateGPUTexture(device, &textureInfo);
	if (!handle)
	{
		SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create texture: %s", SDL_GetError());
		return nullptr;
	}

	SDL_GPUTransferBufferCreateInfo transferBufferInfo = {};
	transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBufferInfo.size = imageSize;

	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

	void* textureTransferPtr = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
	SDL_memcpy(textureTransferPtr, textureData, imageSize);
	SDL_UnmapGPUTransferBuffer(device, transferBuffer);

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	SDL_GPUTextureTransferInfo location = {};
	location.transfer_buffer = transferBuffer;
	location.offset = 0;

	SDL_GPUTextureRegion region = {};
	region.texture = handle;
	region.w = info.width;
	region.h = info.height;
	region.d = 1;

	SDL_UploadToGPUTexture(copyPass, &location, &region, false);

	SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

	SDL_EndGPUCopyPass(copyPass);

	SDL_assert(graphics->numTextures < MAX_TEXTURES);

	Texture* texture = &graphics->textures[graphics->numTextures++];
	texture->handle = handle;
	texture->info = info;

	return texture;
}

void DestroyTexture(Texture* texture)
{
	SDL_ReleaseGPUTexture(device, texture->handle);
}
