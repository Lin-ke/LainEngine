#version 450 core

layout(location = 0) in vec3 vertex_attrib;

void main()
{
    gl_Position = vec4(vertex_attrib, 1.0);
}


