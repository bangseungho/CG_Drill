#version 330 core

out vec4 out_Color; 
uniform mat4 viewTransform;
uniform mat4 projection;

void main(void)
{
	const vec4 vertex[8] = vec4[8] ( vec4(-0.999, -0.999, 0.0, 1.0),
									vec4(0.999, -0.999, 0.0, 1.0),
									vec4(-1.0, 1.0, 0.0, 1.0),
									vec4(1.0, 1.0, 0.0, 1.0),
									vec4(-0.999, -0.999, 0.0, 1.0),
									vec4(-0.999, 0.999, 0.0, 1.0),
									vec4(1.0, 1.0, 0.0, 1.0),
									vec4(1.0, -1.0, 0.0, 1.0));

	const vec4 vColor[8] = vec4[8] (1.0f);

	gl_Position = vertex[gl_VertexID];
	out_Color = vColor[gl_VertexID]; 
}
