#version 330 core

out vec4 FragColor;


in vec2 TexCoord;
in vec3 FragPos;
in vec3 normal;

uniform sampler2D wifi_colors;

uniform float ambient_coeff, diffuse_coeff, spec_coeff;
uniform int shininess;


uniform vec3 viewPos;

#define NR_POINT_LIGHTS 0 
#define NR_DIR_LIGHTS 1


struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct DirLight {
    vec3 direction;

    vec3 color;
};
uniform DirLight dirLights[NR_DIR_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 norm, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.direction);
	// diffuse shading
	float diff = max(dot(norm, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	// attenuation
	//float distance = length(light.position - fragPos);
	//float attenuation = 1.0 / (light.constant + light.linear * distance +
	//	light.quadratic * (distance * distance));
	// combine results
	vec3 ambient = light.color * vec3(texture(wifi_colors, TexCoord));
	vec3 diffuse = light.color * diff * vec3(texture(wifi_colors, TexCoord));
	vec3 specular = light.color * spec * vec3(1, 1, 1);
	//ambient *= attenuation;
	//diffuse *= attenuation;
	//specular *= attenuation;
	return (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}


void main()
{
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 color = vec3(0);
	for (int i = 0; i < NR_DIR_LIGHTS; i++) {
		color += CalcDirLight(dirLights[i], normal, FragPos, viewDir);
	}
	FragColor = vec4(color, 1);
}