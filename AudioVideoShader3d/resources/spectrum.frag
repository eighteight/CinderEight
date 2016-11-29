#version 150
uniform float	resolution;
const float center = 0.5;
const float width = 0.02;

in vec3 vColor;
out vec4 outColor;

uniform sampler2D uVideoTex;

in vec2		TexCoord;

void main(void)
{
	// calculate glowing line strips based on texture coordinate
	float f = fract( resolution * TexCoord.s );
	float d = abs(center - f);
	float strips = clamp(width / d, 0.0, 1.0);

	// calculate fade based on texture coordinate
	float fade = TexCoord.y;

	// calculate output color
    vec4 videoColor = texture(uVideoTex,TexCoord);
    
    outColor.rgb = mix(videoColor.rgb, vColor.rgb, 0.5);//*/ * fade;
	outColor.a = 1.0;
}