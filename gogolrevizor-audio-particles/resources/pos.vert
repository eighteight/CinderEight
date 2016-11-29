//#extension GL_ARB_draw_buffers : enable
varying vec4 texCoord;

void main()
{
	texCoord = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}