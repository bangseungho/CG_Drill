#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 modelTransform; 
out vec3 out_Color;
out vec2 TexCoord;

void main()
{
	gl_Position = vec4(2, 2, 1, 1) * vec4(aPos, 1.0);
	out_Color = aColor;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
