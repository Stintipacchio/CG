// Fragment shader: Toon shading
// ================

#version 450 core

in vec3 N;
in vec3 L;
in vec3 E;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight{
    vec3 position;
    vec3 color;
    float power;
};

uniform Material material;
uniform PointLight light;

void main() {
	
	vec3 ambient = material.ambient; 
	
    vec3 norm = normalize(N);
    vec3 lightDir = normalize(L);

    float diff = max(dot(norm, lightDir), 0.0);
    float diffuse_factor = smoothstep(0.0, 0.8, diff);
    vec3 diffuse = light.power * light.color * diffuse_factor * material.diffuse;

    vec3 viewDir = normalize(E);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular =  light.power * light.color * spec * material.specular;

	vec4 result;	
	float intensity = dot(normalize(L), N);

	if (intensity > 0.95)
		result = vec4(1.0,0.5,0.5,1.0);
	else if (intensity > 0.5)
		result = vec4(0.6,0.3,0.3,1.0);
	else if (intensity > 0.25)
		result = vec4(0.4,0.2,0.2,1.0);
	else
		result = vec4(0.2,0.1,0.1,1.0);
    
    FragColor = result;
}