//uniform sampler2D dbgPos;
uniform sampler2DRect dbgSy;
//uniform sampler2D dbgVel;
//uniform sampler2D dbgInfo;
//uniform sampler2D dbgNoise;

varying vec4 texCoord;

void main()
{
	//gl_FragColor = vec4(0.5, .0, .0, 1.0) /* + texture2D(dbgPos, texCoord.st) * 1000.0 */+ 0.5 * texture2D(dbgSy, texCoord.st).rgba;// + 0.4 * vec4(1.0, texCoord.s, texCoord.t, 1.0);
	
//	gl_FragColor = vec4(texCoord.s/1000.0, texCoord.t/1000.0, 0.0, 1.0);
	gl_FragColor = vec4(texture2DRect(dbgSy, texCoord.st).rgb, 1.0);
}