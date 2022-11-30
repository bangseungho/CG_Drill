#version 330

out vec4 FragColor; //--- 색상 출력

in vec3 out_Color;

void main(void) 
{
	float ambientLight = 0.5;
	FragColor = vec4( ambientLight * out_Color, 1.0);
}
