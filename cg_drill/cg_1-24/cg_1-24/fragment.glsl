#version 330

out vec4 FragColor; //--- 색상 출력

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float isOn = 1;
in vec3 out_Color;

void main(void) 
{
	float ambientLight = 0.1;
	vec3 ambient = ambientLight * lightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diffuseLight = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffuseLight * lightColor;

	int shininess = 128;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float specularLight = max(dot(viewDir, reflectDir), 0.0);
	specularLight = pow(specularLight, shininess);
	vec3 specular = specularLight * lightColor;

	vec3 result;

	if(isOn == 1)
		result = (ambient + diffuse + specular) * out_Color;
	else if (isOn == 0)
		result = ambient * out_Color;
	
	FragColor = vec4(result, 1.0);
}
