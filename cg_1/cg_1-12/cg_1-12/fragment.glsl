#version 330

out vec4 FragColor; //--- ���� ���

uniform vec3 shape_color;

void main(void) 
{
	FragColor = vec4 (shape_color.x, shape_color.y, shape_color.z, 1.0);
}
