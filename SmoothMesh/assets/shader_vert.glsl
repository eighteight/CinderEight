#version 110

uniform mat4 shadowMatrix;

varying vec3 V;
varying vec3 N;
varying vec4 Q;

void main(void)
{
	// transform vertex into eyespace
	V = (gl_ModelViewMatrix * gl_Vertex).xyz;
	
	// transform normal into eyespace
	N = normalize(gl_NormalMatrix * gl_Normal);
	
	// needed by the shadow map algorithm
	Q = shadowMatrix * gl_ModelViewMatrix * gl_Vertex;
	
	// perform standard pass-thru transformations
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}

