

uint32_t hash3(int x, int y, int z, int px, int py, int pz)
{
	x = (x + px) % px;
	y = (y + py) % py;
	z = (z + pz) % pz;

	uint32_t h = (uint32_t)(x * 374761393 + y * 668265263 + z * 2147483647);
	h = (h ^ (h >> 13)) * 1274126177;
	return h;
}

float hashLinear(float x, float y, float z, int px, int py, int pz, int seed)
{
	int xx = (int)floorf(x);
	int yy = (int)floorf(y);
	int zz = (int)floorf(z);

	float h0 = hash3(hash(seed) + xx, yy, zz, px, py, pz) / (float)UINT32_MAX;
	float h1 = hash3(hash(seed) + xx + 1, yy, zz, px, py, pz) / (float)UINT32_MAX;
	float h2 = hash3(hash(seed) + xx, yy + 1, zz, px, py, pz) / (float)UINT32_MAX;
	float h3 = hash3(hash(seed) + xx + 1, yy + 1, zz, px, py, pz) / (float)UINT32_MAX;
	float h4 = hash3(hash(seed) + xx, yy, zz + 1, px, py, pz) / (float)UINT32_MAX;
	float h5 = hash3(hash(seed) + xx + 1, yy, zz + 1, px, py, pz) / (float)UINT32_MAX;
	float h6 = hash3(hash(seed) + xx, yy + 1, zz + 1, px, py, pz) / (float)UINT32_MAX;
	float h7 = hash3(hash(seed) + xx + 1, yy + 1, zz + 1, px, py, pz) / (float)UINT32_MAX;

	float tx = x - xx;
	float ty = y - yy;
	float tz = z - zz;

	float v0 = mix(h0, h1, tx);
	float v1 = mix(h2, h3, tx);
	float v2 = mix(v0, v1, ty);

	float v3 = mix(h4, h5, tx);
	float v4 = mix(h6, h7, tx);
	float v5 = mix(v3, v4, ty);

	float v6 = mix(v2, v5, tz);

	return v6;
}

static Texture* GenerateCloudCoverage(SDL_GPUCommandBuffer* cmdBuffer)
{
	const int width = 128, height = 128;
	uint8_t* noise = (uint8_t*)SDL_malloc(width * height);

	Random random(12345);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float h = hashLinear(x / 3.0f, y / 3.0f, 0, width / 3, height / 3, 128 / 3, 0);
			float i = hashLinear((float)x, (float)y, 5.12f, width, height, 128, 1);
			float value = h * 0.666f + i * 0.333f;
			noise[x + y * width] = (uint8_t)clamp(roundf(value * 256), 0, 255);
		}
	}

	/*
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int octaves = 3;

			float d = 0;

			float amplitude = 1.0f;
			float frequency = 1.0f;
			float totalAmplitude = 0;
			for (int i = 0; i < octaves; i++)
			{
				float simplex = Simplex2fTiled((float)x, (float)y, frequency, width, height, i);
				d += amplitude * (simplex * 0.5f + 0.5f);
				totalAmplitude += amplitude;

				amplitude *= 0.5f;
				frequency *= 2.0f;
			}

			d = clamp(d / totalAmplitude, 0, 1);

			noise[x + y * width] = (uint8_t)clamp(roundf(d * 256), 0, 255);
		}
	}
	*/

	TextureInfo textureInfo = {};
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = 1;
	textureInfo.numMips = 1;
	textureInfo.numLayers = 1;
	textureInfo.numFaces = 1;

	Texture* texture = LoadTextureFromData(noise, width * height, &textureInfo, cmdBuffer);

	SDL_free(noise);

	return texture;
}

inline int fastfloor(float x)
{
	return x > 0 ? (int)x : (int)x - 1;
}

inline float fade(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

inline float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

float rand3(int x, int y, int z, int px, int py, int pz)
{
	return (hash3(x, y, z, px, py, pz) & 0x7fffffff) / float(0x7fffffff);
}

float Perlin3D_Tiled(float x, float y, float z,
	int px, int py, int pz)
{
	int xi = fastfloor(x);
	int yi = fastfloor(y);
	int zi = fastfloor(z);

	float xf = x - xi;
	float yf = y - yi;
	float zf = z - zi;

	float u = fade(xf);
	float v = fade(yf);
	float w = fade(zf);

	auto grad = [](int h, float x, float y, float z)
		{
			int hmod = h & 15;
			float u = hmod < 8 ? x : y;
			float v = hmod < 4 ? y : (hmod == 12 || hmod == 14 ? x : z);
			return ((hmod & 1) ? -u : u) + ((hmod & 2) ? -v : v);
		};

	int X = xi & 255;
	int Y = yi & 255;
	int Z = zi & 255;

	int h000 = hash3(X, Y, Z, px, py, pz);
	int h100 = hash3(X + 1, Y, Z, px, py, pz);
	int h010 = hash3(X, Y + 1, Z, px, py, pz);
	int h110 = hash3(X + 1, Y + 1, Z, px, py, pz);
	int h001 = hash3(X, Y, Z + 1, px, py, pz);
	int h101 = hash3(X + 1, Y, Z + 1, px, py, pz);
	int h011 = hash3(X, Y + 1, Z + 1, px, py, pz);
	int h111 = hash3(X + 1, Y + 1, Z + 1, px, py, pz);

	float x1 = lerp(grad(h000, xf, yf, zf),
		grad(h100, xf - 1, yf, zf), u);
	float x2 = lerp(grad(h010, xf, yf - 1, zf),
		grad(h110, xf - 1, yf - 1, zf), u);

	float y1 = lerp(x1, x2, v);

	float x3 = lerp(grad(h001, xf, yf, zf - 1),
		grad(h101, xf - 1, yf, zf - 1), u);
	float x4 = lerp(grad(h011, xf, yf - 1, zf - 1),
		grad(h111, xf - 1, yf - 1, zf - 1), u);

	float y2 = lerp(x3, x4, v);

	return lerp(y1, y2, w);
}

float Worley3D_Tiled(float x, float y, float z,
	int px, int py, int pz)
{
	int xi = fastfloor(x);
	int yi = fastfloor(y);
	int zi = fastfloor(z);

	float minDist = 1e9f;

	for (int zoff = -1; zoff <= 1; zoff++)
		for (int yoff = -1; yoff <= 1; yoff++)
			for (int xoff = -1; xoff <= 1; xoff++)
			{
				int cx = xi + xoff;
				int cy = yi + yoff;
				int cz = zi + zoff;

				float rx = rand3(cx, cy, cz, px, py, pz);
				float ry = rand3(cx + 31, cy + 17, cz + 13, px, py, pz);
				float rz = rand3(cx + 57, cy + 91, cz + 73, px, py, pz);

				float fx = (cx + rx) - x;
				float fy = (cy + ry) - y;
				float fz = (cz + rz) - z;

				float d = fx * fx + fy * fy + fz * fz;
				if (d < minDist) minDist = d;
			}

	return SDL_sqrtf(minDist);
}

float PerlinWorley3D_TiledFBM(float x, float y, float z,
	float loopX, float loopY, float loopZ, int octaves)
{
	float px = x / loopX * 8.0f;
	float py = y / loopY * 8.0f;
	float pz = z / loopZ * 8.0f;

	float perlin = 0;
	{
		float amplitude = 1.0f;
		float frequency = 1.0f;
		float totalAmplitude = 0;

		for (int i = 0; i < octaves; i++)
		{
			float noise = Perlin3D_Tiled(px * frequency, py * frequency, pz * frequency,
				(int)loopX,
				(int)loopY,
				(int)loopZ);
			noise = noise * 0.5f + 0.5f;

			perlin += amplitude * noise;
			totalAmplitude += amplitude;

			amplitude *= 0.5f;
			frequency *= 2.0f;
		}

		perlin = clamp(perlin / totalAmplitude, 0, 1);
	}

	float worley = 0;
	{
		float amplitude = 1.0f;
		float frequency = 1.0f;
		float totalAmplitude = 0;

		for (int i = 0; i < octaves; i++)
		{
			float noise = Worley3D_Tiled(px * frequency, py * frequency, pz * frequency,
				(int)loopX,
				(int)loopY,
				(int)loopZ);

			worley += amplitude * noise;
			totalAmplitude += amplitude;

			amplitude *= 0.5f;
			frequency *= 2.0f;
		}

		worley = clamp(worley / totalAmplitude, 0, 1);
	}

	// classic cloud-style blend
	//SDL_assert(worley <= 1 && worley >= 0);
	return perlin * 0.5f + (1 - worley) * 0.5f; //clamp(remap(perlin, 0.5f * worley, 1, 0, 1), 0, 1);
}

float Worley3D_TiledFBM(float x, float y, float z,
	float loopX, float loopY, float loopZ, int octaves)
{
	float px = x / loopX * 8.0f;
	float py = y / loopY * 8.0f;
	float pz = z / loopZ * 8.0f;

	float worley = 0;
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float totalAmplitude = 0;

	for (int i = 0; i < octaves; i++)
	{
		float noise = Worley3D_Tiled(px * frequency, py * frequency, pz * frequency,
			(int)loopX,
			(int)loopY,
			(int)loopZ);

		worley += amplitude * noise;
		totalAmplitude += amplitude;

		amplitude *= 0.5f;
		frequency *= 2.0f;
	}

	worley = clamp(worley / totalAmplitude, 0, 1);

	return worley;
}

static Texture* GenerateCloudLowFrequency(SDL_GPUCommandBuffer* cmdBuffer)
{
	const int width = 256, height = 256, depth = 64;
	uint8_t* noise = (uint8_t*)SDL_malloc(width * height * depth);

	Random random(12345);

	for (int z = 0; z < depth; z++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				float h = hashLinear(x / 3.0f, y / 3.0f, z / 3.0f, width / 3, height / 3, depth / 3, 0);
				float i = hashLinear((float)x, (float)y, (float)z, width, height, depth, 1);
				float value = h * 0.666f + i * 0.333f;
				//float value = PerlinWorley3D_TiledFBM(x * 1.5f, y * 1.5f, z * 1.5f, width, height, depth, 4);
				noise[x + y * width + z * width * height] = (uint8_t)clamp(roundf(value * 256), 0, 255);
			}
		}
	}

	TextureInfo textureInfo = {};
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = depth;
	textureInfo.numMips = 1;
	textureInfo.numLayers = 1;
	textureInfo.numFaces = 1;

	Texture* texture = LoadTextureFromData(noise, width * height * depth, &textureInfo, cmdBuffer);

	SDL_free(noise);

	return texture;
}

static Texture* GenerateCloudHighFrequency(SDL_GPUCommandBuffer* cmdBuffer)
{
	const int width = 32, height = 32, depth = 32;
	uint8_t* noise = (uint8_t*)SDL_malloc(width * height * depth);

	for (int z = 0; z < depth; z++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int octaves = 3;
				float worley = Worley3D_TiledFBM((float)x, (float)y, (float)z, width, height, depth, octaves);

				noise[x + y * width + z * width * height] = (uint8_t)clamp(roundf(worley * 256), 0, 255);
			}
		}
	}

	TextureInfo textureInfo = {};
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = depth;
	textureInfo.numMips = 1;
	textureInfo.numLayers = 1;
	textureInfo.numFaces = 1;

	Texture* texture = LoadTextureFromData(noise, width * height * depth, &textureInfo, cmdBuffer);

	SDL_free(noise);

	return texture;
}
