uniform sampler2D shadedModelImage;
uniform sampler2D silhouetteImage;
uniform vec4 silhouetteColor;

void main(void){
	vec2 st = gl_TexCoord[0].st;
	// mix-function for linear blending like this: x * (1.0 - a) + y*a
	gl_FragColor = mix(
		texture2D(shadedModelImage, st), // x = basecolor
		silhouetteColor, // y = color to blend into
		texture2D(silhouetteImage, st).b // a = factor of blending
	);
}