#version 150

uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;

in vec2		ciTexCoord0;
in vec3		ciNormal;
in vec4		ciColor;

out highp vec2	TexCoord;
out lowp vec4	Color;
out highp vec3	Normal;

uniform float		uTexOffset;
uniform sampler2D	uLeftTex;
uniform sampler2D	uRightTex;

const float tenLogBase10 = 3.0102999566398;

layout (std140) uniform FormulaParams {
	float	uA1, uA2;
	float	uB1, uB2;
	float	uM1, uM2;
	float	uN11, uN12;
	float	uN21, uN22;
	float	uN31, uN32;
};

float superformula( float o, float a, float b, float m, float n1, float n2, float n3 )
{
	return pow(pow(abs(cos(m * o / 4)) / a, n2) + pow(abs(sin(m * o / 4)) / b, n3), 1 / n1);
}

float superformulaDeriv( float o, float a, float b, float m, float n1, float n2, float n3 )
{
	float subExpA = n3*m/4*pow(1/b,n3)*sin(m*o/4)*cos(m*o/4)*pow(abs(sin(m*o/4)),n3-2);
	float subExpB = n2*m/4*pow(1/a,n2)*sin(m*o/4)*cos(m*o/4)*pow(abs(cos(m*o/4)),n2-2);
	float subExpC = pow(1/a,n2)*pow(abs(cos(m*o/4)),n2) + pow(1/b,n3) * pow(abs(sin(m*o/4)),n3);
	
	return (1.0f / n1) * (subExpA - subExpB ) * pow( subExpC, 1.0 / n1 - 1.0 );
}

vec3 superformulaNormal( float theta, float phi, float rTheta, float rPhi )
{
	vec3 derivT, derivP;

	float rThetaPrime = superformulaDeriv( theta, uA1, uB1, uM1, uN11, uN21, uN31 );
	float rPhiPrime = superformulaDeriv( phi, uA2, uB2, uM2, uN12, uN22, uN32 );

	// deriv theta
	derivT.x = -( cos(phi) * ( cos(theta) * rThetaPrime + rTheta * sin(theta) ) ) / ( rPhi * rTheta * rTheta );
	derivT.y = ( cos(phi) * ( rTheta * cos(theta) - sin(theta) * rThetaPrime ) ) / ( rPhi * rTheta * rTheta );
	derivT.z = 0.0;
	// deriv phi
	derivP.x = -( cos(theta) * ( cos(phi) * rPhiPrime + rPhi * sin(phi) ) ) / ( rPhi * rPhi * rTheta );
	derivP.y = -( sin(theta) * ( cos(phi) * rPhiPrime + rPhi * sin(phi) ) ) / ( rPhi * rPhi * rTheta );
	derivP.z = ( rPhi * cos(phi) - sin(phi) * rPhiPrime ) / ( rPhi * rPhi );
	
	return cross( normalize(derivT), normalize(derivP) );
}

void main( void )
{
    // retrieve texture coordinate and offset it to scroll the texture
    vec2 coord = ciTexCoord0.st + vec2(0.0, uTexOffset+100.0);
    
    // retrieve the FFT from left and right texture and average it
    float fft = max(0.0001, mix( texture( uLeftTex, coord ).r, texture( uRightTex, coord ).r, 0.5));
    
    // convert to decibels
    float decibels = 0.1 * log( fft ) * tenLogBase10;

	float theta = mix( -3.1415926535, 3.1415926535, ciTexCoord0.s )* decibels; // theta domain = [-pi,pi]
	float phi = mix( -3.1415926535 / 2.0, 3.1415926535 / 2.0, ciTexCoord0.t )* decibels; // phi domain = [-pi/2,pi/2]
	float rTheta = superformula( theta, uA1, uB1, uM1, uN11, uN21, uN31 )* decibels;
	float rPhi = superformula( phi, uA2, uB2, uM2, uN12, uN22, uN32 )* decibels;
	float x = cos(theta) * cos(phi) / rTheta / rPhi;
	float y = sin(theta) * cos(phi) / rTheta / rPhi;
	float z = sin(phi) / rPhi;

	vec3 position = vec3( x, y , z ) ;
	vec3 normal = superformulaNormal( theta, phi, rTheta, rPhi );

	gl_Position	= ciModelViewProjection * vec4( position, 1.0 );
	Normal		= ciNormalMatrix * normal;
	Color 		= ciColor;
	TexCoord	= ciTexCoord0;
}
