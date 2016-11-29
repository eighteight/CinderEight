#version 120

// Screen size
uniform float width;
uniform float height;

// Textures
uniform sampler2D srcTexture;
uniform sampler2D texture;

void main(void)
{

	// Get coordinates and pixel sizes
	vec2 texCoord = gl_TexCoord[0].st;
	float w = 1.0 / width;
	float h = 1.0 / height;

	// Calculate offsets
	float x = texture2D(texture, texCoord + vec2(0.0, -h)).g - texture2D(texture, texCoord + vec2(w, 0.0)).b;
	float y = texture2D(texture, texCoord + vec2(0.0, -h)).r - texture2D(texture, texCoord + vec2(0.0, h)).r;

	// Amplify refraction
	x *= x * y;

	// Get color from image texture
	vec4 srcColor = texture2D(srcTexture, texCoord + vec2(x, y));

	// Floor
	float s = texture2D(texture, texCoord).g * 0.2;

	// Set color
	gl_FragColor = vec4(srcColor.r + x - s, srcColor.g + x - s, srcColor.b - y - s, 1.0);

}