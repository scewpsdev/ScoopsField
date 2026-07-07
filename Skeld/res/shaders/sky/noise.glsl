// Hash by David_Hoskins
#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uvec2(UI0, UI1)
#define UI3 uvec3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

vec3 hash33(vec3 p)
{
	uvec3 q = uvec3(ivec3(p)) * UI3;
	q = (q.x ^ q.y ^ q.z)*UI3;
	return -1. + 2. * vec3(q) * UIF;
}

// Gradient noise by iq (modified to be tileable)
float gradientNoise(vec3 x, vec3 freq)
{
    // grid
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    // quintic interpolant
    vec3 u = w * w * w * (w * (w * 6. - 15.) + 10.);

    
    // gradients
    vec3 ga = hash33(mod(p + vec3(0., 0., 0.), freq));
    vec3 gb = hash33(mod(p + vec3(1., 0., 0.), freq));
    vec3 gc = hash33(mod(p + vec3(0., 1., 0.), freq));
    vec3 gd = hash33(mod(p + vec3(1., 1., 0.), freq));
    vec3 ge = hash33(mod(p + vec3(0., 0., 1.), freq));
    vec3 gf = hash33(mod(p + vec3(1., 0., 1.), freq));
    vec3 gg = hash33(mod(p + vec3(0., 1., 1.), freq));
    vec3 gh = hash33(mod(p + vec3(1., 1., 1.), freq));
    
    // projections
    float va = dot(ga, w - vec3(0., 0., 0.));
    float vb = dot(gb, w - vec3(1., 0., 0.));
    float vc = dot(gc, w - vec3(0., 1., 0.));
    float vd = dot(gd, w - vec3(1., 1., 0.));
    float ve = dot(ge, w - vec3(0., 0., 1.));
    float vf = dot(gf, w - vec3(1., 0., 1.));
    float vg = dot(gg, w - vec3(0., 1., 1.));
    float vh = dot(gh, w - vec3(1., 1., 1.));
	
    // interpolation
    return va + 
           u.x * (vb - va) + 
           u.y * (vc - va) + 
           u.z * (ve - va) + 
           u.x * u.y * (va - vb - vc + vd) + 
           u.y * u.z * (va - vc - ve + vg) + 
           u.z * u.x * (va - vb - ve + vf) + 
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

// Tileable 3D worley noise
float worleyNoise(vec3 uv, vec3 freq)
{    
    vec3 id = floor(uv);
    vec3 p = fract(uv);
    
    float minDist = 10000.;
    for (float x = -1.; x <= 1.; ++x)
    {
        for(float y = -1.; y <= 1.; ++y)
        {
            for(float z = -1.; z <= 1.; ++z)
            {
                vec3 offset = vec3(x, y, z);
            	vec3 h = hash33(mod(id + offset, freq)) * .5 + .5;
    			h += offset;
            	vec3 d = p - h;
           		minDist = min(minDist, dot(d, d));
            }
        }
    }
    
    // inverted worley noise
    return 1. - minDist;
}



// A standard pseudo-random 3D hashing function
vec3 hash3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453123) * 2.0 - 1.0;
}

// Seamless 3D Perlin Gradient Noise
float seamlessPerlin(vec3 p, float tileSize) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);

    // Force grid vertices to wrap smoothly at the texture boundaries
    vec3 i0 = mod(i, vec3(tileSize));
    vec3 i1 = mod(i + vec3(1.0), vec3(tileSize));

    return mix(mix(mix(dot(hash3(i0), f - vec3(0,0,0)), dot(hash3(vec3(i1.x, i0.y, i0.z)), f - vec3(1,0,0)), u.x),
                   mix(dot(hash3(vec3(i0.x, i1.y, i0.z)), f - vec3(0,1,0)), dot(hash3(vec3(i1.x, i1.y, i0.z)), f - vec3(1,1,0)), u.x), u.y),
               mix(mix(dot(hash3(vec3(i0.x, i0.y, i1.z)), f - vec3(0,0,1)), dot(hash3(vec3(i1.x, i0.y, i1.z)), f - vec3(1,0,1)), u.x),
                   mix(dot(hash3(vec3(i0.x, i1.y, i1.z)), f - vec3(0,1,1)), dot(hash3(i1), f - vec3(1,1,1)), u.x), u.y), u.z) * 0.5 + 0.5;
}

// Seamless 3D Worley Cellular Noise
float seamlessWorley(vec3 p, float tileSize) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    float minDist = 1.0;

    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec3 neighbor = vec3(float(x), float(y), float(z));
                // Wrap lattice grid coordinates cleanly
                vec3 wrappedVertex = mod(i + neighbor, vec3(tileSize));
                
                // Extract a predictable cell center point position
                vec3 cellPosition = hash3(wrappedVertex) * 0.5 + 0.5; 
                vec3 diff = neighbor + cellPosition - f;
                
                minDist = min(minDist, length(diff));
            }
        }
    }
    return clamp(minDist, 0.0, 1.0);
}



// Fbm for Perlin noise based on iq's blog
float perlinFbm(vec3 p, float freq, int octaves, float lacunarity, float persistence)
{
    float amplitude = 1;
    float frequency = freq;
    float noise = 0;
    float sum = 0;

    for (int i = 0; i < octaves; i++)
    {
        noise += amplitude * seamlessPerlin(p * frequency, frequency);
        sum += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
        p += 12345.6789;
    }

    return noise / sum;

    /*
    float G = exp2(-.85);
    float amp = 1.;
    float noise = 0.;
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * (gradientNoise(p * freq, freq) * 0.5 + 0.5);
        freq *= 2.;
        amp *= G;
        p += 12345.6789;
    }
    
    return noise;
    */
}

float perlinFbm(vec3 p, float freq, int octaves)
{
    return perlinFbm(p, freq, octaves, 2, 0.5);
}

// Tileable Worley fbm inspired by Andrew Schneider's Real-Time Volumetric Cloudscapes
// chapter in GPU Pro 7.
float worleyFbm(vec3 p, float freq, int octaves, float lacunarity, float persistence)
{
    float amplitude = 1;
    float frequency = freq;
    float noise = 0;
    float sum = 0;

    for (int i = 0; i < octaves; i++)
    {
        noise += amplitude * seamlessWorley(p * frequency, frequency);
        sum += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
        p += 12345.6789;
    }

    return noise / sum;

    //return worleyNoise(p*freq, freq) * .625 +
    //    	 worleyNoise(p*freq*2., freq*2.) * .25 +
    //    	 worleyNoise(p*freq*4., freq*4.) * .125;
}

float worleyFbm(vec3 p, float freq, int octaves)
{
    return worleyFbm(p, freq, octaves, 2, 0.5);
}
