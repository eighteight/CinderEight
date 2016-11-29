#version 120
varying float depth;

void main(void){
	//gl_Position = gl_ModelViewProjectionMatrix *  gl_Vertex;
	gl_Position = ftransform();
	depth = gl_Position.z;
}