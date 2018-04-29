#version 100

uniform sampler2D mCamTex;
uniform sampler2D mLoopTex;

uniform highp float B;
varying highp vec2	Vertex;

void main( void )
{
    //highp float displace = dot(texture2D(texCam, Vertex).rgb, vec3(0.3, 0.6, 0.1))*0.1;
    //gl_FragColor = texture2D(texDisplace, Vertex + vec2(displace));
    gl_FragColor = mix(texture2D(mCamTex, Vertex ),texture2D(mLoopTex, Vertex), 0.5);
    //gl_FragColor = texture2D(mLoopTex, Vertex );

}
