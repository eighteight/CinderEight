
#version 410

uniform vec2		pixel;		 // Size of a pixel in [0,0]-[1,0]
uniform sampler2D	texBuffer;   // Data texture
uniform sampler2D	texRefract;  // Refraction texture (the image to be warped)

in vec2		vTexCoord;			 // Texture coordinate

out vec4    FragColor;

void main( void )
{
	// Calculate refraction
	vec2 above		= texture( texBuffer, vTexCoord + vec2( 0.0, -pixel.y ) ).rg;
	float x			= above.g - texture( texBuffer, vTexCoord + vec2( pixel.x, 0.0 ) ).g;
	float y			= above.r - texture( texBuffer, vTexCoord + vec2( 0.0, pixel.y ) ).r;

	// Sample the texture from the target position
	FragColor	= texture( texRefract, vTexCoord + vec2( x, y ) );
}
