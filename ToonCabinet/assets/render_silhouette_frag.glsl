#version 120
uniform sampler2D depthEdgeImage;
uniform sampler2D normalEdgeImage;
uniform float textureSizeX;
uniform float textureSizeY;

// access maximum edge value of depth and normal image
float getMaximumEdgeValue(vec2 st){
	vec2 texcoord = clamp(st, 0.001, 0.999);
	vec4 d = texture2D(depthEdgeImage, texcoord);
	vec4 n = texture2D(normalEdgeImage, texcoord);
	return max(d.r, n.g);
}

void main(void){
	float dxtex = 1.0 / textureSizeX;
	float dytex = 1.0 / textureSizeY;

	vec2 st = gl_TexCoord[0].st;
	// define size of dilation
	int scope = 1;
	for(int s = -scope; s <= scope; s++){
		for(int t = -scope; t <= scope; t++){
			vec2 offset = vec2(s*dxtex, t*dytex);
			// check for edge
			if(getMaximumEdgeValue(st + offset) > 0.1){
				// is silhouette => color pixel blue
				gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
				return;
			}
		}
	}
	// no silhouette => color pixel black
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}