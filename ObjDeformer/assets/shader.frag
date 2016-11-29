#version 150
#extension GL_ARB_explicit_attrib_location : enable
layout(location = 0) out vec4 attachment0;
in vec4		Color;
in vec3		Normal;
uniform float time;
uniform float A, B;
out vec4 	oColor;
uniform sampler2D	mTextPinPong;
in vec2	TexCoord;

void main( void )
{
    vec2 coord = TexCoord;
    
    // retrieve previous texture
    vec4 prevColor = texture( mTextPinPong, coord );
    vec3 normal = normalize( -Normal );
	float diffuse = max( dot( normal, vec3( 0, 0, -1 ) ), 0 );

    //oColor = prevColor * A + Color * B * diffuse;
    attachment0 = prevColor * A + Color * B * diffuse;
    //oColor = Color * diffuse;//clamp(diffuse + time,0.0, 1.0);

}