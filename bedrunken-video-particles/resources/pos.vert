//#extension GL_ARB_draw_buffers : enable
varying vec4 texCoord;
uniform bool doParticles;
void main()
{
    if (doParticles){
        texCoord = gl_MultiTexCoord0;
    } else {
        texCoord = gl_MultiTexCoord1;
    }
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}