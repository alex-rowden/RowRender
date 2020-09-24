#version 330 core
out vec4 FragColor;

in vec2 TexCoords;


uniform sampler2D normal_tex;
uniform sampler2D albedo_tex;
uniform sampler2D fragPos_tex;
uniform sampler2D depth;
uniform sampler2D ssao_tex;

uniform float ambient_coeff, diffuse_coeff, spec_coeff;
uniform int shininess;
uniform int num_point_lights;

#define NR_POINT_LIGHTS 120 
//#define NR_DIR_LIGHTS 0

float LinearizeDepth()
{
	float zNear = 0.1;    // TODO: This should probably be tunable
	float zFar = 1000.0; // TODO: This should probably be tunable
	float depth = texture2D(depth, TexCoords).x;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

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

uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform vec3 viewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
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
	vec3 ambient = light.ambient * color * texture(ssao_tex, TexCoords).x;
	vec3 diffuse =  light.diffuse * diff * color;
	vec3 specular = light.specular * spec * vec3(1, 1, 1);
	//ambient *= attenuation;
	//diffuse *= attenuation;
	//specular *= attenuation;
	return light.constant * (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
{
	vec3 lightDir = normalize(light.direction);
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
	vec3 ambient = light.color * color;
	vec3 diffuse =  light.color * diff * color;
	vec3 specular = light.color * spec * vec3(1, 1, 1);
	//ambient *= attenuation;
	//diffuse *= attenuation;
	//specular *= attenuation;
	return (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}

void main()
{
	vec3 norm = texture(normal_tex, TexCoords).rgb * 2 - 1;
	float stencil = texture(normal_tex, TexCoords).a;
	vec3 FragPos = texture(fragPos_tex, TexCoords).rgb;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 color = vec3(0,0,0);
	for (int i = 0; i < num_point_lights; i++) {
		color += CalcPointLight(pointLights[i], norm, FragPos, viewDir, texture(albedo_tex, TexCoords).rgb);
	}for (int i = 0; i < 0; i++) {
		//color += CalcDirLight(dirLights[i], norm, FragPos, viewDir);
	}
	FragColor = vec4(color, 1);
}