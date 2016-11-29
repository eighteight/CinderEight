uniform float scale;

varying vec4 texCoord;

void main()
{
	//vec4 newVertexPos;
	//newVertexPos = vec4(scale * gl_MultiTexCoord0.x, scale * gl_MultiTexCoord0.y, 1.0, 1.0);
	texCoord = gl_MultiTexCoord0;
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor  = gl_Color;
}