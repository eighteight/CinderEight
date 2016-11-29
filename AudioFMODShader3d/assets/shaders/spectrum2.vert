#version 110

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;

void main(void)
{
    // retrieve texture coordinate and offset it to scroll the texture
    vec2 coord = gl_MultiTexCoord0.st + vec2(0.0, uTexOffset);
    
    // retrieve the FFT from left and right texture and average it
    float fft = max(0.0001, mix( texture2D( uLeftTex, coord ).r, texture2D( uRightTex, coord ).r, 0.5));
    
    // convert to decibels
	const float logBase10 = 0.30102999566398; // 1.0 / log(10.0);
    float decibels = 10.0 * log( fft ) * logBase10;
    
    // offset the vertex based on the decibels and create a cylinder
    const float two_pi = 6.2831853;
    float fade = gl_MultiTexCoord0.t;
    float r = 100.0 + decibels;
    vec4 vertex = gl_Vertex;
    vertex.y = r * cos(gl_MultiTexCoord0.t * two_pi);
    vertex.z = r * sin(gl_MultiTexCoord0.t * two_pi);
    
    // pass (unchanged) texture coordinates, bumped vertex and vertex color
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
    gl_FrontColor = gl_Color;
}