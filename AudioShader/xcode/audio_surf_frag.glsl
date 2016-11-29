// by nikos papadopoulos, 4rknova / 2013
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

uniform vec3		iResolution;           // viewport resolution (in pixels)
uniform float		iGlobalTime;           // shader playback time (in seconds)
//uniform float		iChannelTime[4];       // channel playback time (in seconds)
//uniform vec3		iChannelResolution[4]; // channel resolution (in pixels)
//uniform vec4		iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D	iChannel0;          // input channel. XX = 2D/Cube
//uniform vec4		iDate;                 // (year, month, day, time in seconds)

#ifdef GL_ES
precision highp float;
#endif

#define PI  3.14159
#define EPS .001

#define T .03  // Thickness
#define W 2.   // Width
#define A .09  // Amplitude
#define V 1.   // Velocity

void main(void)
{
	vec2 c = gl_FragCoord.xy / iResolution.xy;
	vec4 s = texture2D(iChannel0, c * .5);
	c = vec2(0., A*s.y*sin((c.x*W+iGlobalTime*V)* 2.5)) + (c*2.-1.);
	float g = max(abs(s.y/(pow(c.y, 2.1*sin(s.x*PI))))*T,
				  abs(.1/(c.y+EPS)));
	gl_FragColor = vec4(g*g*s.y*.6, g*s.w*.44, g*g*.7, 1.);
}