#version 330

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0

layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1

out vec3 out_Color;

uniform float xMove;

void main(void) 
{
	gl_Position = vec4 (in_Position.x + xMove, in_Position.y, in_Position.z, 1.0);
	out_Color = in_Color;
}