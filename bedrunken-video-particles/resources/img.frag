uniform sampler2D imgTexture;
uniform float transper;

void main(void)
{

	// Get coordinates
	vec2 texCoord = gl_TexCoord[0].st;
    
	// Get texture color
	vec4 color = texture2D(imgTexture, texCoord);
	
	gl_FragColor = vec4(color.r, color.g, color.b, transper);
    
}
