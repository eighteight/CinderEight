uniform sampler2D depthTex;
uniform sampler2D soundTex;
uniform float waveAmplitude;
varying vec4 vVertex;
varying float depth;
varying float audioAmpl;


void main()
{
	gl_TexCoord[0]		= gl_MultiTexCoord0;
	vVertex				= vec4( gl_Vertex );
	
	depth				= texture2D( depthTex, gl_TexCoord[0].st ).b;
    
    audioAmpl           = texture2D(soundTex, gl_TexCoord[0].st).r;
	
	vVertex.z			+= depth * 1000.0;
    
    vVertex.z			+= audioAmpl*waveAmplitude;
    
    vVertex.y			+= audioAmpl*waveAmplitude;

	gl_Position			= gl_ModelViewProjectionMatrix * vVertex;
}
