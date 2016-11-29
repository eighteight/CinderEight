uniform sampler2D displacementMap;
uniform sampler2D mFrameTexture;
uniform bool doParticles;
void main()
{
    if (doParticles){
        float scale = 400.0;
        vec4 newVertexPos;
        vec4 dv = texture2D(displacementMap, gl_MultiTexCoord0.xy );
        newVertexPos = vec4(scale*dv.x, scale*dv.y, scale*dv.z, 1);
        gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
    } else {
        gl_TexCoord[1] = gl_MultiTexCoord1;
        gl_Position = ftransform();
    }
}