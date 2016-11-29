#version 120
uniform sampler2D depthImage;
uniform float textureSizeX;
uniform float textureSizeY;

int weights[25];

vec4 getDepth(vec2 st){
	vec2 texcoord = clamp(st, 0.001, 0.999);
	return texture2D(depthImage, texcoord);
}

void main(void){
	float dxtex = 1.0 / textureSizeX;
	float dytex = 1.0 / textureSizeY;

	vec2 st = gl_TexCoord[0].st;
    vec4 total = vec4(0.0);

	total = -1*getDepth(st + vec2(0.0, -dytex)) + -1*getDepth(st + vec2(-dxtex, 0.0)) + 4*getDepth(st) + -1*getDepth(st + vec2(dxtex, 0.0)) + -1*getDepth(st + vec2(0.0, dytex));

	gl_FragColor = total;
}