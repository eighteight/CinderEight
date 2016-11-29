uniform sampler2D texPos;
uniform sampler2D texInf;

uniform float scale;
varying float age;
uniform float partSize;

varying float doDiscard;
uniform float numParts;

void main()
{
	vec4 newVertexPos;
	vec4 dv;
	
	doDiscard = 0.0;
	
	dv = texture2D( texPos, gl_MultiTexCoord0.st );
    vec3 info = texture2D(texInf, gl_MultiTexCoord0.st).rgb;
	age = info.r;
	
	doDiscard = step(pow(1.0-numParts, 0.4), info.b);
	
    //scale vertex position to screen size
	newVertexPos = vec4(scale * dv.x, scale * dv.y, scale * dv.z, 1);
	
    //adjust point size, increasing size kills performance
	gl_PointSize = partSize * 10.0 * age;// * info.b * 4.0;
	
	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}