#version 110

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;
uniform float time;
const float tenLogBase10 = 3.0102999566398;
void main(void)
{	
	// retrieve texture coordinate and offset it to scroll the texture
	vec2 coord = gl_MultiTexCoord0.st + vec2(0.0, uTexOffset+100.0);

	// retrieve the FFT from left and right texture and average it
	float fft = max(0.0001, mix( texture2D( uLeftTex, coord ).r, texture2D( uRightTex, coord ).r, 0.5));

	// convert to decibels
	float decibels = log( fft ) * tenLogBase10;

	// offset the vertex based on the decibels
	vec4 vertex = gl_Vertex;
	vertex.y += 20.0 * decibels;

	// pass (unchanged) texture coordinates, bumped vertex and vertex color
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
    gl_FrontColor = gl_Color;
}