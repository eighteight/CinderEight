#version 110
varying float depth;

void main()
{

	gl_FragColor.rgb	= 10.0*vec3(depth,depth,depth);
	gl_FragColor.a		= 1.0;
}
