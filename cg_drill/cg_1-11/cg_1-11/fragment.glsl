#version 330

in vec3 out_Color;

out vec4 FragColor; //--- ���� ���
//uniform vec3 new_Color;

void main(void) 
{
	FragColor = vec4 (out_Color, 1.0);
}
