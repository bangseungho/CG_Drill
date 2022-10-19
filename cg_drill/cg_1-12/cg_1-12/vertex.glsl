#version 330

layout (location = 0) in vec3 vPos;

uniform mat4 modelTransform;

void main()
{
	gl_Position = modelTransform * vec4(vPos, 1.0);
}