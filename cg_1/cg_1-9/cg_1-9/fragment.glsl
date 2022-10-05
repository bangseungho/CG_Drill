#version 330

uniform vec3 select_Color;

out vec4 FragColor; //--- 색상 출력

void main(void) 
{
	FragColor = vec4 (select_Color, 1.0);
}
