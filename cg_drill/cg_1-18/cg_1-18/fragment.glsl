#version 330

out vec4 FragColor; //--- 색상 출력

in vec4 out_Color;

void main(void) 
{
	FragColor = vec4(out_Color.x, out_Color.y, out_Color.z, 1.0);
}
