#version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 vColor;

uniform mat4 obj1_modelTransform;
uniform mat4 viewTransform;
uniform mat4 projection;

out vec3 out_Color;
out vec3 FragPos;
out vec3 Normal;

void main()
{
	out_Color = vColor;

	FragPos = vec3(obj1_modelTransform * vec4(vPos, 1.0));
	Normal = mat3(transpose(inverse(obj1_modelTransform))) * aNormal;


	gl_Position = projection * viewTransform * vec4(FragPos, 1.0);
}
