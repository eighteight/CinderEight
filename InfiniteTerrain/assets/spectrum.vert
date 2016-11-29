#version 150 core

in vec4 ciPosition;
in vec3 ciColor;

uniform mat4 ciModelView;

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;
const float logBase10 = 0.30102999566398;
const float two_pi = 6.2831853;
out vec3 vColor;

void main(void)
{	
	// retrieve texture coordinate and offset it to scroll the texture
	vec2 coord = ciPosition.xy  + vec2(0.0, uTexOffset);


	// retrieve the FFT from left and right texture and average it
	//float fft = max(0.0001, mix( texture2D( uLeftTex, coord ).r, texture2D( uRightTex, coord ).r, 10.5));
    float fft = max(0.0001, mix( ciColor.r, ciColor.r, 10.5));
    
	// convert to decibels
	float decibels = 10.0 * log( fft ) * logBase10;

	// offset the vertex based on the decibels
	vec4 vertex = ciPosition;
	vertex.y += 4.0 * decibels;
    
    float r = 200.0;
    vertex.z = r * sin(ciPosition.t * two_pi) + vertex.x;
    //vertex.x = r * sin(gl_MultiTexCoord0.t * two_pi) + vertex.x;

	// pass (unchanged) texture coordinates, bumped vertex and vertex color
//	gl_TexCoord[0] = ciPosition;
    
//	gl_Position = gl_ModelViewProjectionMatrix * vertex;
    gl_Position = ciModelView * ciPosition;
	vColor = ciColor;
}