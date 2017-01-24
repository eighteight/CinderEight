#version 100
uniform highp float	resolution;
const highp float center = 0.5;
const highp float width = 0.02;
uniform sampler2D	uTex0;
uniform highp float time;
uniform bool isSphere;

varying highp vec2 vVertex;


void main(void)
{
	// calculate glowing line strips based on texture coordinate
	highp float f = fract( resolution * vVertex.s );
	highp float d = abs(center - f);
	highp float strips = clamp(width / d, 0.0, 1.0);

	// calculate fade based on texture coordinate
	highp float fade = vVertex.y;

	// calculate output color
    highp vec3 rgb = texture2D( uTex0, vVertex.st ).rgb;
    highp vec4 color;
    if (false && time < 1.0) {
        highp vec3 gray = vec3(dot(rgb, vec3(0.299,0.587,0.114)));
        highp vec3 ave = rgb * time + gray * (1.0 - time);
        color.rgb = ave;
    } else {
        color.rgb = rgb;
    }
    color.a = 1.;
    
    gl_FragColor = color;
    
}
