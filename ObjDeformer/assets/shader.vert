#version 150

uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;
uniform mat3	ciModelViewMatrix;
uniform bool    isTwist;

in vec4		ciPosition;
in vec2		ciTexCoord0;
in vec3		ciNormal;
in vec4		ciColor;

uniform float time;

out lowp vec4	Color;
out highp vec3	Normal;

const float two_pi = 6.2831853;
const float deg_to_rad = 0.01745327777778;
const float angle_deg_max = 4000.0;

out vec2 TexCoord;
    
vec4 DoTwist( vec4 pos, float t )
{
    float st = sin(t);
    float ct = cos(t);
    vec4 new_pos;
    
    new_pos.x = pos.x*ct - pos.z*st;
    new_pos.z = pos.x*st + pos.z*ct;
    
    new_pos.y = pos.y;
    new_pos.w = pos.w;
    
    return( new_pos );
}


void main(void)
{
    if (isTwist) {
        float angle_deg = angle_deg_max*sin(time);
        float angle_rad = angle_deg * deg_to_rad;
        
        float ang = (0.5 + ciPosition.y) * angle_rad;
        vec4 twistedPosition = DoTwist(ciPosition, ang);
        vec4 twistedNormal = DoTwist(vec4(ciNormal, ang), ang);

        gl_Position = ciModelViewProjection * twistedPosition;
        Normal		= ciNormalMatrix * twistedNormal.xyz;
    } else {
        gl_Position = ciModelViewProjection * ciPosition;
        Normal		= ciNormalMatrix * ciNormal;
    }
    float r = clamp(time + ciColor.r, 0.0, 1.0);
    float g = clamp(time + ciColor.g, 0.0, 1.0);
    float b = clamp(time + ciColor.b, 0.0, 1.0);
    Color = vec4(r , g , b , 1.0);
    
    TexCoord = ciTexCoord0;
}
