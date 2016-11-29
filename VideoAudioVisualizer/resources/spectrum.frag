#version 110
uniform float	resolution;
const float center = 0.5;
const float width = 0.02;
uniform sampler2D	uVideoTex;
uniform float time;
void main(void)
{
	// calculate glowing line strips based on texture coordinate
	float f = fract( resolution * gl_TexCoord[0].s );
	float d = abs(center - f);
	float strips = clamp(width / d, 0.0, 1.0);

	// calculate fade based on texture coordinate
	float fade = gl_TexCoord[0].y;

	// calculate output color
    vec3 rgb = texture2D(uVideoTex,gl_TexCoord[0].xy).rgb *fade;// * strips;// * fade;
    
    if (time < 1.0) {
        vec3 gray = vec3(dot(rgb, vec3(0.299,0.587,0.114)));
        vec3 ave = rgb * time + gray * (1.0 - time);
        gl_FragColor.rgb = ave;
    } else {
        gl_FragColor.rgb = rgb;
    }
    gl_FragColor.a = 1.0;
}