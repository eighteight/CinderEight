#version 150 core

// Shader Inputs
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if LMB down), zw: click
uniform vec4      iDate;                 // (year, month, day, time in seconds)

out vec4 oColor;
#define PI 3.141592653

// a circular ripple around center, with n cycles between center and span
float ripple( vec2 center, float cycles, float span, float offset )
{
	float d = distance( gl_FragCoord.xy, center );
	return cos( (d * cycles * 2.0 * PI / span) + offset );
}

void main(void)
{
		float half_width = iResolution.x * 0.5;
    float r1 = ripple( iMouse.xy, 8.0, half_width, -iGlobalTime );
    float r2 = ripple( iMouse.zw, 8.0, half_width, -iGlobalTime );
    float gray = (r1 + r2) * 0.8;
    oColor = vec4( vec3( gray ), 1.0 );
}
