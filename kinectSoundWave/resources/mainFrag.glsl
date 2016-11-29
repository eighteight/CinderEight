#version 110
varying float depth;
varying float audioAmpl;

void main()
{
	if( audioAmpl < 0.1 ) discard;
	gl_FragColor.rgb	= vec3(depth,1.0-depth,depth);
	gl_FragColor.a		= 1.0;
}





