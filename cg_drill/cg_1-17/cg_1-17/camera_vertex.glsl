#version 330 core

layout (location = 0) in vec3 vPos;
uniform mat4 modelTransform;
uniform mat4 viewTransform;

void main()
{
	gl_Position = viewTransform * modelTransform * vec4(vPos, 1.0);
}