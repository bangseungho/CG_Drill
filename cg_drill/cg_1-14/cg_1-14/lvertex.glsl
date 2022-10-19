#version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;

uniform mat4 lmodelTransform;
out vec3 out_Color;

void main()
{
	out_Color = vColor;
	gl_Position = lmodelTransform * vec4(vPos, 1.0);
}