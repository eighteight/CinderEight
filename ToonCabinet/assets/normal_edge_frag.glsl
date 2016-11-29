#version 120
uniform sampler2D normalImage;
uniform float textureSizeX;
uniform float textureSizeY;
uniform float normalEdgeThreshold;

vec3 getNormal(vec2 st){
	vec2 texcoord = clamp(st, 0.001, 0.999);
	return texture2D(normalImage, texcoord).rgb;
}

void main(void){
	float dxtex = 1.0 / textureSizeX;
	float dytex = 1.0 / textureSizeY;

	vec2 st = gl_TexCoord[0].st;
	// access center pixel and 4 surrounded pixel
	vec3 center = getNormal(st).rgb;
	vec3 left = getNormal(st + vec2(dxtex, 0.0)).rgb;
	vec3 right = getNormal(st + vec2(-dxtex, 0.0)).rgb;
	vec3 up = getNormal(st + vec2(0.0, -dytex)).rgb;
	vec3 down = getNormal(st + vec2(0.0, dytex)).rgb;

	// discrete Laplace operator
	vec3 laplace = abs(-4*center + left + right + up + down);
	// if one rgb-component of convolution result is over threshold => edge
	if(laplace.r > normalEdgeThreshold
		|| laplace.g > normalEdgeThreshold
		|| laplace.b > normalEdgeThreshold){
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); // => color the pixel green
	} else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // black
	}
}