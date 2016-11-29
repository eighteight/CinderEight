uniform sampler2D displacementMap;
void main()
{
	float scale = 400.0;
	vec4 newVertexPos;
	vec4 dv = texture2D( displacementMap, gl_MultiTexCoord0.xy );
	newVertexPos = vec4(scale*dv.x, scale*dv.y, scale*dv.z, 1);
	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
}