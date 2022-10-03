#version 330

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0

uniform vec3 move;
uniform vec3 scale;

void main(void) 
{
	gl_Position = vec4(in_Position.xyz * scale + move, 1.0);
}