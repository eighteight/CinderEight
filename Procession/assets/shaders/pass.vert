#version 100
uniform mat4 ciModelViewProjection;

attribute vec4		ciPosition;
attribute vec2		ciTexCoord0;
attribute vec4     ciColor;
varying vec2	Vertex;
varying vec4    Color;

void main( void ) 
{
	Vertex.xy 	= ciTexCoord0;
    Color = ciColor;
	gl_Position = ciModelViewProjection * ciPosition;
}
