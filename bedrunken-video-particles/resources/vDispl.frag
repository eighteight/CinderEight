#extension GL_ARB_texture_rectangle : enable
uniform sampler2D displacementMap;
uniform sampler2DRect mFrameTexture;
uniform bool doParticles;
uniform bool isNegative;

void main(void)
{
    // Get coordinates
	vec2 texCoord = doParticles ? gl_TexCoord[0].st : gl_TexCoord[1].st;
    //vec4 color = 

    //vec4 speed =	gl_FragData[1];
    float col = isNegative ? 1.0 : 0.0;
       //vec4 speed =	gl_FragData[1];
    gl_FragColor = vec4(col, col, col, 0.0);

}