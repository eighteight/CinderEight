#version 120 
#extension GL_EXT_geometry_shader4 : enable
uniform float amp;
uniform float phase;
uniform float scale;
uniform float speed;
uniform float width;
uniform vec2 mouse;
uniform float aspect;
// Inputs
uniform float rotation;
uniform float size;

	
// Kernel
void main(void)
{   float aspect1 = 2.;
    vec4 vert0 = vec4(cos(radians(315.0)) * size, sin(radians(315.0)) * size*aspect, 0.0, 0.0);
    vec4 vert1 = vec4(cos(radians(225.0)) * size, sin(radians(225.0)) * size*aspect, 0.0, 0.0);
    vec4 vert2 = vec4(cos(radians(135.0)) * size, sin(radians(135.0)) * size*aspect, 0.0, 0.0);
    vec4 vert3 = vec4(cos(radians(45.0)) * size, sin(radians(45.0)) * size*aspect, 0.0, 0.0);


	// Iterate through vertices
	for (int i = 0; i < gl_VerticesIn; i++)
	{
        // Draw the left triangle
        gl_Position = gl_PositionIn[0] + vert0;
        EmitVertex();
        gl_Position = gl_PositionIn[0] + vert1;
        EmitVertex();
        gl_Position = gl_PositionIn[0] + vert3;
        EmitVertex();
        
        // Close this triangle
        EndPrimitive();
        
        // And now the right one
        gl_Position = gl_PositionIn[0] + vert3;
        EmitVertex();
        gl_Position = gl_PositionIn[0] + vert1;
        EmitVertex();
        gl_Position = gl_PositionIn[0] + vert2;
        EmitVertex();
        
        // Close the second triangle to form a quad
        EndPrimitive();
	}

	// Close shape (implied -- this line is not necessary
	// if you are only drawing one shape)
	EndPrimitive();

}
