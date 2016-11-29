#version 120
varying float depth;

void main(void){
	float depthColor = 1 - depth / 10000.0;
	gl_FragColor = vec4(depthColor, depthColor, 0.0, 1.0);
}