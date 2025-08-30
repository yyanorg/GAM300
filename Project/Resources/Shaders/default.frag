#version 330 core
struct Material {
   sampler2D diffuse0; // Diffuse map, helps to determine the texture and color of each pixel
   sampler2D specular0; // Specular map, helps to determine the shininess of each pixel, e.g wood has almost no shininess
   float shininess;
};
in vec2 TexCoords;
uniform Material material;

// ------------------Direction-------------------
struct DirectionLight{
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
uniform DirectionLight dirLight;

vec3 calculateDirectionLight(DirectionLight light, vec3 normal, vec3 view_direction)
{
   vec3 light_direction = normalize(-light.direction);

   // Diffuse shading
   float diff = max(dot(normal, light_direction), 0.0);

   // Specular shading
   vec3 reflect_direction = reflect(-light_direction, normal);
   float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess);

   vec3 ambient  = light.ambient  * vec3(texture(material.diffuse0, TexCoords));
   vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse0, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular0, TexCoords));
   return (ambient + diffuse + specular);
}

// ------------------Point-------------------
struct PointLight{
   vec3 position;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;

   float constant;
   float linear;
   float quadratic;   
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 view_direction)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(view_direction, reflectDir), 0.0), material.shininess);
   // attenuation
   float distance    = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // combine results
   vec3 ambient  = light.ambient  * vec3(texture(material.diffuse0, TexCoords));
   vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse0, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular0, TexCoords));
   ambient  *= attenuation;
   diffuse  *= attenuation;
   specular *= attenuation;
   return (ambient + diffuse + specular);
}

// ------------------Spotlight-------------------
struct Spotlight{
   vec3 position;  
   vec3 direction;
   float cutOff;
   float outerCutOff;
  
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
	
   float constant;
   float linear;
   float quadratic;
};
uniform Spotlight spotLight;

vec3 calculateSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 view_direction)
{
   vec3 light_direction = normalize(light.position - fragPos);

   // diffuse
   float diff = max(dot(normal, light_direction), 0.0);
   
   // spec
   vec3 reflect_direction = reflect(-light_direction, normal);
   float spec = pow(max(dot(view_direction, light_direction), 0.0), material.shininess);

   // attenuation
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

   // spotlight intensity
   float theta = dot(light_direction, normalize(-light.direction));
   float epsilon = light.cutOff - light.outerCutOff;
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

   vec3 ambient = light.ambient * vec3(texture(material.diffuse0, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse0, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular0, TexCoords));
   ambient *= attenuation * intensity;
   diffuse *= attenuation * intensity;
   specular *= attenuation * intensity;
   return (ambient + diffuse + specular);
}

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 cameraPos;

// function prototypes
vec3 calculateDirectionLight(DirectionLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calculateSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
   // properties
   vec3 norm = normalize(Normal);
   vec3 viewDir = normalize(cameraPos - FragPos);
    
   // == =====================================================
   // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
   // For each phase, a calculate function is defined that calculates the corresponding color
   // per lamp. In the main() function we take all the calculated colors and sum them up for
   // this fragment's final color.
   // == =====================================================
   // phase 1: directional lighting
   vec3 result = calculateDirectionLight(dirLight, norm, viewDir);
   // phase 2: point lights
   for(int i = 0; i < NR_POINT_LIGHTS; i++)
     result += calculatePointLight(pointLights[i], norm, FragPos, viewDir);    
   // phase 3: spot light
   result += calculateSpotlight(spotLight, norm, FragPos, viewDir);    
    
   FragColor = vec4(result, 1.0);
}