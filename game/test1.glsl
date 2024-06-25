// draw triangle vertex shader
layout(location = 0) in vec3 vertex;

int main()
{
    gl_Position = vec4(vertex, 1.0);
}
