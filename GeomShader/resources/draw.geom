#version 120 
#extension GL_EXT_geometry_shader4 : enable

// Inputs
uniform float rotation;
uniform float size;
	
// Kernel
void main(void)
{

	// Iterate through vertices
	for (int i = 0; i < gl_VerticesIn; i++)
	{
 
		// Point A
		gl_Position = gl_PositionIn[i];
		EmitVertex();

		// Point B
		gl_Position.x += cos(gl_Position.y * rotation) * size;
		gl_Position.y += sin(gl_Position.y * rotation) * size;
		EmitVertex();

		// Point C
		gl_Position.x -= cos(gl_Position.y * rotation) * size;
		gl_Position.y -= sin(gl_Position.y * rotation) * size;
		EmitVertex();

        // Point d
		gl_Position.x -= cos(gl_Position.y * rotation) * size;
		gl_Position.y -= sin(gl_Position.y * rotation) * size;
		EmitVertex();
	}

	// Close shape (implied -- this line is not necessary
	// if you are only drawing one shape)
	EndPrimitive();

}
