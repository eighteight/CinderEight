// Inputs
uniform float amp;
uniform float phase;
uniform float rotation;
uniform float scale;
uniform float size;
uniform float speed;
uniform float width;

// Kernel
void main(void)
{

	// Position vertex from time
	vec4 vertex = gl_Vertex;
	float wave = (sin((phase * speed) + vertex.x * width)) * (amp * scale);
	gl_Position = gl_ModelViewProjectionMatrix * vec4(scale * vertex.x, scale * vertex.y + wave, scale * vertex.z - wave, 1.0);

}
