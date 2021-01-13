#version 150 core
// Author @patriciogv - 2015
// http://patriciogonzalezvivo.com
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
//uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)
// Author @patriciogv - 2015
// http://patriciogonzalezvivo.com


out vec4 oColor;


float random(in float x){
    return fract(sin(x)*43758.5453);
}

float random(in vec2 st){
    return fract(sin(dot(st.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float randomChar(in vec2 outer,in vec2 inner){
    float grid = 5.;
    vec2 margin = vec2(.2,.05);
    float seed = 23.;
    vec2 borders = step(margin,inner)*step(margin,1.-inner);
    return step(.5,random(outer*seed+floor(inner*grid))) * borders.x * borders.y;
}

vec3 matrix(in vec2 st){
    float rows = 50.0;
    vec2 ipos = floor(st*rows)+vec2(1.,0.);

    ipos += vec2(.0,floor(iGlobalTime*20.*random(ipos.x)));

    vec2 fpos = fract(st*rows);
    vec2 center = (.5-fpos);

    float pct = random(ipos);
    float glow = (1.-dot(center,center)*3.)*2.0;

    return vec3(randomChar(ipos,fpos) * pct * glow);
}

void main( void ){
    vec2 st = gl_FragCoord.xy / iResolution.xy;

    st.y *= iResolution.y/iResolution.x;

    oColor = vec4(matrix(st), 1.0);
}
