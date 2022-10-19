#version 330

out vec4 FragColor; //--- 색상 출력

uniform vec4 out_Color;

void main(void) 
{
	FragColor = out_Color;
}
