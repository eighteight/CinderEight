#version 120
varying vec3 normal;

void main(void){
	// colorize pixel with normal vector used as color
	gl_FragColor.rgb = normal.xyz;
}