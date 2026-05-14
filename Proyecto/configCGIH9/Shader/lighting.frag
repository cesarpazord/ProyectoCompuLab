#version 330 core

#define NUMBER_OF_POINT_LIGHTS 1

struct Material
{
    vec3 color;
    float shininess;
    bool useTexture; // <-- Nuestro interruptor
};

struct DirLight { vec3 direction; vec3 ambient; vec3 diffuse; vec3 specular; };
struct PointLight { vec3 position; float constant; float linear; float quadratic; vec3 ambient; vec3 diffuse; vec3 specular; };
struct SpotLight { vec3 position; vec3 direction; float cutOff; float outerCutOff; float constant; float linear; float quadratic; vec3 ambient; vec3 diffuse; vec3 specular; };

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords; // <-- Recibimos las UVs

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

// ESTA ES TU IMAGEN:
uniform sampler2D texture_diffuse1; 

// Prototipos
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor );

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // ¿Usamos textura o color sólido?
    vec3 baseColor;
    if(material.useTexture) {
        baseColor = vec3(texture(texture_diffuse1, TexCoords));
    } else {
        baseColor = material.color;
    }

    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor);

    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor);

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, baseColor);

    color = vec4(result, 1.0);
}

// ==========================
// DIR LIGHT
// ==========================
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;
    return (ambient + diffuse + specular);
}

// ==========================
// POINT LIGHT
// ==========================
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;

    return (ambient + diffuse + specular) * attenuation;
}

// ==========================
// SPOT LIGHT
// ==========================
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;

    return (ambient + diffuse + specular) * attenuation * intensity;
}