#version 330

in vec3 in_Position; //--- ��ġ ����: attribute position 0

in vec3 in_Color; //--- �÷� ����: attribute position 1

void main(void) 
{
	gl_Position = vec4 (in_Position.x, in_Position.y, in_Position.z, 1.0);
}