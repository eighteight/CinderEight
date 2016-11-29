#version 120
uniform sampler2D depthImage;
uniform float textureSizeX;
uniform float textureSizeY;
uniform float depthEdgeThreshold;

float getDepth(vec2 st){
	vec2 texcoord = clamp(st, 0.001, 0.999);
	return texture2D(depthImage, texcoord).r;
}

void main(void){
	float dxtex = 1.0 / textureSizeX;
	float dytex = 1.0 / textureSizeY;

	vec2 st = gl_TexCoord[0].st;
	// access center pixel and 4 surrounding pixels
	float center = getDepth(st);
	float left = getDepth(st + vec2(dxtex, 0.0));
	float right = getDepth(st + vec2(-dxtex, 0.0));
	float up = getDepth(st + vec2(0.0, -dytex));
	float down = getDepth(st + vec2(0.0, dytex));

	// discrete Laplace operator
	float laplace = abs(-4 * center + left + right + up + down);
	// if result of convolution is over threshold => there is an edge
	if(laplace > depthEdgeThreshold){
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); // color the pixel white
	} else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // black
	}
}