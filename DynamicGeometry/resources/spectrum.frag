#version 150
uniform float	resolution;
const float center = 0.5;
const float width = 0.02;
uniform sampler2D	uTex0;
uniform float time;
uniform bool isSphere;

out vec4 oFragColor;
in VertexData	{
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
} vVertexIn;


void main(void)
{
	// calculate glowing line strips based on texture coordinate
	float f = fract( resolution * vVertexIn.texCoord.s );
	float d = abs(center - f);
	float strips = clamp(width / d, 0.0, 1.0);

	// calculate fade based on texture coordinate
	float fade = vVertexIn.texCoord.y;

	// calculate output color
    vec3 rgb = texture( uTex0, vVertexIn.texCoord.st ).rgb;
    vec4 color;
    if (false && time < 1.0) {
        vec3 gray = vec3(dot(rgb, vec3(0.299,0.587,0.114)));
        vec3 ave = rgb * time + gray * (1.0 - time);
        color.rgb = ave;
    } else {
        color.rgb = rgb;
    }
    color.a = 1.;
    
    oFragColor = color;
    
}