#version 150

in vec3 in_Position;
uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
    gl_Position = projectionMatrix * modelviewMatrix * vec4(in_Position, 1.0); // This should include projection
}
