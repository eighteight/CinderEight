#version 120
varying vec3 normal;

void main(void){
	//normal = ((gl_NormalMatrix * gl_Normal).xyz + 1.0) / 2.0;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}