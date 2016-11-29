
///https://github.com/MasDennis/Rajawali/wiki/Tutorial-23-Custom-Vertex-Shader

#version 110

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;
const float two_pi = 6.2831853;
const float tenLogBase10 = 3.0102999566398; // 10.0 / log(10.0);
uniform float time;


const vec3 cXaxis = vec3(1.0, 0.0, 0.0);
const vec3 cYaxis = vec3(0.0, 1.0, 0.0);
const vec3 cZaxis = vec3(0.0, 0.0, 1.0);
const float cStrength = 0.5;

const float sphere_radius = 1.0;

const float flat_width = 200.0;
const float flat_height = 200.0;
const float flatness = 0.0;

void main(void)
{
    vec3 directionVec = normalize(vec3(gl_MultiTexCoord0));
    float xangle = dot(cXaxis, directionVec) * 5.0;
    float yangle = dot(cYaxis, directionVec) * 6.0;
    float zangle = dot(cZaxis, directionVec) * 4.5;
    vec4 timeVec = gl_MultiTexCoord0;
    //float time = 0.0;
    float cosx = cos(time + xangle);
    float sinx = sin(time + xangle);
    float cosy = cos(time + yangle);
    float siny = sin(time + yangle);
    float cosz = cos(time + zangle);
    float sinz = sin(time + zangle);

    timeVec.x += directionVec.x * cosx * siny * cosz * cStrength;
    timeVec.y += directionVec.y * sinx * cosy * sinz * cStrength;
    timeVec.z += directionVec.z * sinx * cosy * cosz * cStrength;
    
    //gl_Position = gl_ModelViewProjectionMatrix * timeVec;
    
    
    vec3 sphere_position = gl_Vertex.xyz * sphere_radius;
    vec3 flat_position = vec3 (vec2 (flat_width, flat_height) * (gl_MultiTexCoord0.xy - vec2 (0.5, 0.5)), 0.0);
    
    
    
    
    
    
    // retrieve texture coordinate and offset it to scroll the texture
    vec2 coord = gl_MultiTexCoord0.st + vec2(0.0, uTexOffset);
    
    // retrieve the FFT from left and right texture and average it
    float fft = max(0.0001, mix( texture2D( uLeftTex, coord ).r, texture2D( uRightTex, coord ).r, 0.5));
    
    // convert to decibels
    float decibels = tenLogBase10 * log( fft );
    
    // offset the vertex based on the decibels and create a cylinder
    float fade = gl_MultiTexCoord0.t;
    float r = 100.0 + decibels;
    vec4 vertex = gl_Vertex;
    vertex.y = r * cos(gl_MultiTexCoord0.t * two_pi) - 0.25 * vertex.y;
    vertex.z = r * sin(gl_MultiTexCoord0.t * two_pi) - time * vertex.z;

    vec4 lerp_position = vec4 (mix (sphere_position, flat_position, flatness), 1.0);
    
    // pass (unchanged) texture coordinates, bumped vertex and vertex color
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(sphere_position, 1.0);
    gl_FrontColor = gl_Color;
}