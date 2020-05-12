#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_diffuse4;
uniform sampler2D texture_diffuse5;
uniform sampler2D texture_diffuse6;
uniform sampler2D texture_diffuse7;
uniform sampler2D texture_diffuse8;
uniform sampler2D texture_diffuse9;
uniform sampler2D texture_diffuse10;
uniform sampler2D texture_diffuse11;
uniform sampler2D texture_diffuse12;
uniform sampler2D texture_diffuse13;
uniform sampler2D texture_diffuse14;
uniform sampler2D texture_diffuse15;

uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;
uniform sampler2D texture_specular4;
uniform sampler2D texture_specular5;
uniform sampler2D texture_specular6;
uniform sampler2D texture_specular7;
uniform sampler2D texture_specular8;
uniform sampler2D texture_specular9;
uniform sampler2D texture_specular10;
uniform float ambient_coeff, diffuse_coeff, spec_coeff;
uniform int shininess;

#define NR_POINT_LIGHTS 2
struct PointLight {
	vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	// attenuation
	//float distance = length(light.position - fragPos);
	//float attenuation = 1.0 / (light.constant + light.linear * distance +
	//	light.quadratic * (distance * distance));
	// combine results
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(1, 1, 1);
	//ambient *= attenuation;
	//diffuse *= attenuation;
	//specular *= attenuation;
	return (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 output = vec3(0,0,0);
	for (int i = 0; i < NR_POINT_LIGHTS; i++) {
		output += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	}
	FragColor = vec4(output, 1.0);
}