#version 100

uniform sampler2D   mCamTex;
varying highp vec2	Vertex;
varying highp vec4  Color;

void main( void )
{
    highp float displace = dot(Color.rgb, vec3(0.3, 0.6, 0.1))*0.1;
    gl_FragColor = mix(texture2D(mCamTex, Vertex + vec2(displace)), Color, 0.1);

//    highp float displace = dot(texture2D(mCamTex, Vertex).rgb, vec3(0.3, 0.6, 0.1))*0.1;
//    highp vec4 tmp = mix(Color, texture2D(mCamTex, Vertex + vec2(displace)), 0.5);
//    
//    displace = 1.0 - displace;
//    gl_FragColor = vec4(tmp.rgb, displace*15.0);

}
