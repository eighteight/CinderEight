
#version 410

uniform mat4 ciModelViewProjection;


in vec4 ciPosition;
in vec2 ciTexCoord0;

in vec4 ciColor;

out vec2 vTexCoord;
out vec4 vColor;


void main()
{
    vColor = ciColor;
    vTexCoord = ciTexCoord0;
    gl_Position = ciModelViewProjection * ciPosition;
}
