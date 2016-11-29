#version 150

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;
const float tenLogBase10 = 3.0102999566398;

in vec4 ciPosition;
in vec3 ciColor;
in vec2	ciTexCoord0;
in vec3	ciNormal;

uniform mat4 ciModelViewProjection;
out vec3 vColor;
out highp vec2	TexCoord;


void main(void)
{
    TexCoord = ciTexCoord0;
    
	// retrieve texture coordinate and offset it to scroll the texture
	vec2 coord = ciTexCoord0.st + vec2(0.0, uTexOffset+100.0);

	// retrieve the FFT from left and right texture and average it
	float fft = max(0.0001, mix( texture( uLeftTex, coord ).r, texture( uRightTex, coord ).r, 0.5));

	// convert to decibels
	float decibels = log( fft ) * tenLogBase10;

	// offset the vertex based on the decibels
    vec4 vertex = ciPosition;//gl_Vertex;
	vertex.y += 2.0 * decibels;
    
    gl_Position = ciModelViewProjection * vertex;//ciPosition;
    vColor = ciColor;
//
//	// pass (unchanged) texture coordinates, bumped vertex and vertex color
//	gl_TexCoord[0] = gl_MultiTexCoord0;
//	gl_Position = gl_ModelViewProjectionMatrix * vertex;
//	gl_FrontColor = gl_Color;
}