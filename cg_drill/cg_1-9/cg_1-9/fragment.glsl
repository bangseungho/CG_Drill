#version 330

uniform vec3 select_Color;

out vec4 FragColor; //--- ���� ���

void main(void) 
{
	FragColor = vec4 (select_Color, 1.0);
}
