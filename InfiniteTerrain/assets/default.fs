#version 150 core

// Shader Inputs
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if LMB down), zw: click
uniform vec4      iDate;                 // (year, month, day, time in seconds)

out vec4 oColor;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    oColor = vec4( uv, 0.5 + 0.5 * sin(iGlobalTime), 1.0 );
}
