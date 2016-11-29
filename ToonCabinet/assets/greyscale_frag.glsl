#version 110

uniform sampler2D tex0;

void main()
{
	float gray = dot(texture2D(tex0, gl_TexCoord[0].st).rgb, vec3(0.299, 0.587, 0.114));
    
	gl_FragColor = vec4(gray, gray, gray, 1.0);
}
