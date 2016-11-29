#version 410

uniform vec2		pixel;						// Size of a pixel in [0,0]-[1,0]
uniform sampler2D	texBuffer;					// Data texture

in	vec2		vTexCoord;							// Texture coordinate
in vec4         vColor;
out vec4        FragColor;

const float			dampen	= 0.993;			// Ripple dampening
const float			power	= 1.5;				// Input power
const float			speed	= 1.0;				// Ripple travel speed

// Samples velocity from neighbor
float getSpring( float height, vec2 position, float factor ) 
{
	return ( texture( texBuffer, position ).r - height ) * factor;
}

void main( void ) 
{
	// Kernel size
	vec2 kernel		= pixel * speed;

	// Sample the color to get the height and velocity of this pixel
    vec4 color		= texture( texBuffer, vTexCoord );
	float height	= color.r;
	float vel		= color.g;

	// Sample neighbors to update this pixel's velocity. Sampling inside of for() loops
	// is very slow, so we write it all out.
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  2.0,  3.0 ), 0.0022411859348636983 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  0.0,  3.0 ), 0.0056818181818181820 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -2.0,  3.0 ), 0.0022411859348636983 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  2.0,  2.0 ), 0.0066566640639421000 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  0.0,  2.0 ), 0.0113636363636363640 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -2.0,  2.0 ), 0.0066566640639421000 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  3.0,  1.0 ), 0.0047597860217705710 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  1.0,  1.0 ), 0.0146919683956074150 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -1.0,  1.0 ), 0.0146919683956074150 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -3.0,  1.0 ), 0.0047597860217705710 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  2.0,  0.0 ), 0.0113636363636363640 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -2.0,  0.0 ), 0.0113636363636363640 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  3.0, -1.0 ), 0.0047597860217705710 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  1.0, -1.0 ), 0.0146919683956074150 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -1.0, -1.0 ), 0.0146919683956074150 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -3.0, -1.0 ), 0.0047597860217705710 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  2.0, -2.0 ), 0.0066566640639421000 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  0.0, -2.0 ), 0.0113636363636363640 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -2.0, -2.0 ), 0.0066566640639421000 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  2.0, -3.0 ), 0.0022411859348636983 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2(  0.0, -3.0 ), 0.0056818181818181820 * power );
	vel				+= getSpring( height, vTexCoord + kernel * vec2( -2.0, -3.0 ), 0.0022411859348636983 * power );

	// Update this pixel's height (red channel)
	height			+= vel;
	
	// Reduce the velocity
	vel				*= dampen;

	// Store the height and velocity in the red and green channels
    FragColor = vec4( height, vel, 0.0, 1.0 );
}
