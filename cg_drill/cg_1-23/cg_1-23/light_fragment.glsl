#version 330 core

out vec4 FragColor;
uniform vec3 light_source;

void main()
{
    FragColor = vec4(light_source, 1.0);
}