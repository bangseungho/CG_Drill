#version 330 core

out vec4 out_Color; 
uniform mat4 viewTransform;
uniform mat4 projection;

void main(void)
{
const vec4 vColor[6] = vec4[6] (vec4(1.0, 0.0, 0.0, 1.0),
								vec4(1.0, 0.0, 0.0, 1.0),
								vec4(0.0, 1.0, 0.0, 1.0),
								vec4(0.0, 1.0, 0.0, 1.0),
								vec4(0.0, 0.0, 1.0, 1.0),
								vec4(0.0, 0.0, 1.0, 1.0));

	const vec4 vertex[6] = vec4[6] (vec4(-5.0 , 0.0, 0.0, 1.0),
								vec4(5.0, 0.0, 0.0, 1.0),
								vec4(0.0, 5.0, 0.0, 1.0),
								vec4(0.0, -5.0 ,0.0, 1.0),
								vec4(0.0, 0.0, 5.0, 1.0),
								vec4(0.0, 0.0, -5.0, 1.0));

	gl_Position = projection * viewTransform * vertex[gl_VertexID];
	out_Color = vColor[gl_VertexID]; 
}