#version 330

in vec3 out_Color;

out vec4 FragColor; //--- 색상 출력

void main(void) 
{
	FragColor = vec4 (out_Color, 1.0);
}
