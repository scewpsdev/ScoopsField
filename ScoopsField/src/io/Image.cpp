#include "Image.h"

#include <SDL3/SDL.h>


struct ASTCHeader
{
	Uint8 magic[4];
	Uint8 blockX;
	Uint8 blockY;
	Uint8 blockZ;
	Uint8 dimX[3];
	Uint8 dimY[3];
	Uint8 dimZ[3];
};

struct DDS_PIXELFORMAT {
	int dwSize;
	int dwFlags;
	int dwFourCC;
	int dwRGBBitCount;
	int dwRBitMask;
	int dwGBitMask;
	int dwBBitMask;
	int dwABitMask;
};

struct DDS_HEADER {
	int dwMagic;
	int dwSize;
	int dwFlags;
	int dwHeight;
	int dwWidth;
	int dwPitchOrLinearSize;
	int dwDepth;
	int dwMipMapCount;
	int dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	int dwCaps;
	int dwCaps2;
	int dwCaps3;
	int dwCaps4;
	int dwReserved2;
};

struct DDS_HEADER_DXT10 {
	int dxgiFormat;
	int resourceDimension;
	unsigned int miscFlag;
	unsigned int arraySize;
	unsigned int miscFlags2;
};


void* LoadASTCImage(const char* path, int* pWidth, int* pHeight, int* pImageDataLength)
{
	size_t fileSize;
	void* fileContents = SDL_LoadFile(path, &fileSize);
	if (fileContents == NULL)
	{
		SDL_assert(!"Could not load ASTC image!");
		return NULL;
	}

	ASTCHeader* header = (ASTCHeader*)fileContents;
	if (header->magic[0] != 0x13 || header->magic[1] != 0xAB || header->magic[2] != 0xA1 || header->magic[3] != 0x5C)
	{
		SDL_assert(!"Bad magic number!");
		return NULL;
	}

	// Get the image dimensions in texels
	*pWidth = header->dimX[0] + (header->dimX[1] << 8) + (header->dimX[2] << 16);
	*pHeight = header->dimY[0] + (header->dimY[1] << 8) + (header->dimY[2] << 16);

	// Get the size of the texture data
	unsigned int block_count_x = (*pWidth + header->blockX - 1) / header->blockX;
	unsigned int block_count_y = (*pHeight + header->blockY - 1) / header->blockY;
	*pImageDataLength = block_count_x * block_count_y * 16;

	void* data = SDL_malloc(*pImageDataLength);
	SDL_memcpy(data, (char*)fileContents + sizeof(ASTCHeader), *pImageDataLength);
	SDL_free(fileContents);

	return data;
}

void* LoadDDSImage(const char* path, int* pWidth, int* pHeight, int* pImageDataLength)
{
	size_t fileSize;
	void* fileContents = SDL_LoadFile(path, &fileSize);
	if (fileContents == NULL)
	{
		SDL_assert(!"Could not load DDS image!");
		return NULL;
	}

	DDS_HEADER* header = (DDS_HEADER*)fileContents;
	if (header->dwMagic != 0x20534444)
	{
		SDL_assert(!"Bad magic number!");
		return NULL;
	}

	bool hasDX10Header = header->ddspf.dwFlags == 0x4 && header->ddspf.dwFourCC == 0x30315844;

	*pWidth = header->dwWidth;
	*pHeight = header->dwHeight;
	*pImageDataLength = header->dwPitchOrLinearSize;

	void* data = SDL_malloc(*pImageDataLength);
	SDL_memcpy(data, (char*)fileContents + sizeof(DDS_HEADER) + (hasDX10Header ? sizeof(DDS_HEADER_DXT10) : 0), *pImageDataLength);
	SDL_free(fileContents);

	return data;
}
