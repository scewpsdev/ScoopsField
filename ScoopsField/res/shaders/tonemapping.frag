#version 460

#include "common.glsl"

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_hdrFrame;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;

#define exposure params.x
};


// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat =
{
    {0.59719, 0.07600, 0.02840},
    {0.35458, 0.90834, 0.13383},
    {0.04823, 0.01566, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
{
    { 1.60475, -0.10208, -0.00327},
    {-0.53108,  1.10813, -0.07276},
    {-0.07367, -0.00605,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

// ACES fitted
vec3 acesFitted(vec3 color)
{
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    // Clamp to [0, 1]
    color = clamp(color, 0, 1);

    return color;
}

// CRYENGINE-style Filmic Curve exposure parameters
// Tweak these in your engine to sculpt the perfect look
const float ToeScale = 0.20;      // Controls shadow falloff (higher = deeper blacks)
const float MidtonesScale = 0.55; // Controls the brightness of middle grey
const float ShoulderScale = 0.85; // Controls highlight roll-off (higher = softer peaks)

// Function to map a single channel using the filmic curve
float filmic_curve(float x) {
    float A = 0.22; // Shoulder strength
    float B = 0.30; // Linear strength
    float C = 0.10; // Linear angle
    float D = 0.20; // Toe strength
    float E = 0.01; // Toe numerator
    float F = 0.30; // Toe denominator
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 filmic(vec3 color)
{
    color *= 5;

    // 2. Calculate Luminance (Luma) to prevent hue shifts and oversaturation
    // Standard Rec. 709 weights for perceived brightness
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // 3. Apply the filmic curve to the luminance
    float mappedLuma = filmic_curve(luma);

    // 4. Scale original color ratios by mapped luma to preserve original hues
    vec3 ldrColor = color * (mappedLuma / max(luma, 0.0001));

    // 5. CryEngine Environment Tones (Customization injection points)
    // Scale shadows, midtones, and highlights according to artist preference
    ldrColor = mix(ldrColor, ldrColor * ToeScale, 1.0 - mappedLuma);
    ldrColor = mix(ldrColor, ldrColor * MidtonesScale, mappedLuma * (1.0 - mappedLuma));
    ldrColor = mix(ldrColor, ldrColor * ShoulderScale, mappedLuma);

    // 6. Mimic KCD2's Muted/Earthy Aesthetic (Desaturation Step)
    //float finalLuma = dot(ldrColor, vec3(0.2126, 0.7152, 0.0722));
    //ldrColor = mix(vec3(finalLuma), ldrColor, 0.85); // 15% Desaturation

    return ldrColor;
}

vec3 gammaCorrection(vec3 color)
{
	color = pow(color, vec3(1.0 / 2.2));
	return color;
}

void main()
{
	vec3 color = texture(s_hdrFrame, v_texcoord).rgb;
    color *= exposure;
	//color = linearToSRGB(color);

	color = acesFitted(color);
	color = gammaCorrection(color);

	out_color = vec4(color, 1);
}
