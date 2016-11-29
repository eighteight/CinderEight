// -*- C++ -*- automatisch in C++mode wechseln (emacs)

//void main(void)
//{
//    gl_TexCoord[0] = gl_MultiTexCoord0;
//    gl_Position    = gl_Vertex;
//}

#version 150

in vec4 ciPosition;
in vec3 ciColor;

uniform mat4 ciModelView;

out vec3 vColor;

void main()
{
    gl_Position = ciModelView * ciPosition;
    vColor = ciColor;
}
