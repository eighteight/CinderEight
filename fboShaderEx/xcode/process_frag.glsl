#version 120
#define KERNEL_SIZE 9

// Screen size
uniform float width;
uniform float height;

// Dampening
uniform float dampen;

// Texture
uniform sampler2D texture;

// Reaction-Diffusion
uniform float ru;
uniform float rv;
uniform float f;
uniform float k;

// Offsets
vec2 offset[KERNEL_SIZE];

void main(void)
{

	// Get coordinates and pixel sizes
	vec2 texCoord = gl_TexCoord[0].st;
	float w = 1.0 / width;
	float h = 1.0 / height;
	
	// Set neighbor locations
	offset[0] = vec2(-w, -h);
	offset[1] = vec2(0.0, -h);
	offset[2] = vec2(w, -h);
	offset[3] = vec2(-w, 0.0);
	offset[4] = vec2(0.0, 0.0);
	offset[5] = vec2(w, 0.0);
	offset[6] = vec2(-w, h);
	offset[7] = vec2(0.0, h);
	offset[8] = vec2(w, h);

	// Find sum of neighbors and self
	float sumR = 0.0;
	float sumB = 0.0;
	for (int n = 0; n < KERNEL_SIZE; n++)
	{
		sumR += texture2D(texture, texCoord + offset[n]).r;
		sumB += texture2D(texture, texCoord + offset[n]).b;
	}

	// Use average to determine new value
	float u = sumR / float(KERNEL_SIZE);
	float v = texture2D(texture, texCoord).g;

	// Tweak values a tad
	u += u / dampen + 0.2;
	v += 0.2;

	// Reaction diffusion
	float r = texture2D(texture, texCoord).r;
	float F = f + r * 0.025 - 0.0005;
	float K = k + r * 0.025 - 0.0005;
	float uvv = u * v * v;
	float du = ru * sumR - uvv + F * (1.0 - u);
	float dv = rv * sumB + uvv - (F + K) * v;
	u += (du / dampen) * 0.6;
	v += dv * 0.6;

	// Clamp values
	u = 1.0 - clamp(u, 0.0, 1.0);
	v = 1.0 - clamp(v, 0.0, 1.0);

	// Set color
	gl_FragColor = vec4(u, 1.0 - u / v, v, 0.0);

}