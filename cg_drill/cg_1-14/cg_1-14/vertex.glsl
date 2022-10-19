#version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;

uniform mat4 modelTransform;
out vec3 out_Color;

void main()
{
	out_Color = vColor;
	gl_Position = modelTransform * vec4(vPos, 1.0);
}