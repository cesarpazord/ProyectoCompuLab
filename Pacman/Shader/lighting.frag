#version 330 core

in vec3 FragPos;
in vec3 Normal;
out vec4 color;

// ---- Estructuras de luz ----
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3  position;
    float constant;
    float linear;
    float quadratic;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
};

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
};

struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};

// ---- Uniforms ----
uniform DirLight    dirLight;
uniform PointLight  pointLights[4];
uniform SpotLight   spotLight;
uniform Material    material;
uniform vec3        viewPos;
uniform int         transparency;   // 1 = usa alpha parcial

// ---- Prototipos ----
vec3 CalcDirLight  (DirLight light,   vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight (SpotLight light,  vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    for (int i = 0; i < 4; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    float alpha = (transparency == 1) ? 0.6 : 1.0;
    color = vec4(result, alpha);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float dist  = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float dist  = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    float theta   = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular) * attenuation * intensity;
}
