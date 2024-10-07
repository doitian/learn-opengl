#version 330 core

#define POINT_LIGHTS_COUNT 4
#define NEARLY_ZERO 0.00001

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};
struct DirLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
};
struct SpotLight {
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
  float cutOff;
  float outerCutOff;
};

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform SpotLight spotLight;
uniform vec3 viewPos;

vec3 calcDirLight(DirLight light, vec3 norm, vec3 materialAmbient, vec3 materialSpecular) {
  // ambient
  vec3 ambient = light.ambient * materialAmbient;

  // diffuse
  vec3 lightDir = normalize(-light.direction);
  vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.diffuse * materialAmbient;

  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * light.specular * materialSpecular;

  return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 norm, vec3 materialAmbient, vec3 materialSpecular) {
  float distance = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  if(attenuation > NEARLY_ZERO) {
    return calcDirLight(DirLight(FragPos - light.position, light.ambient, light.diffuse, light.specular), norm, materialAmbient, materialSpecular) * attenuation;
  }

  return vec3(0.0);
}

vec3 calcSpotLight(SpotLight light, vec3 norm, vec3 materialAmbient, vec3 materialSpecular) {
  vec3 lightDir = normalize(light.position - FragPos);
  float theta = dot(lightDir, normalize(-light.direction));
  float intensity = 0.0;
  if(theta > light.outerCutOff) {
    float epsilon = light.cutOff - light.outerCutOff;
    intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
  }

  if(intensity > NEARLY_ZERO) {
    return intensity * calcPointLight(PointLight(light.position, light.ambient, light.diffuse, light.specular, light.constant, light.linear, light.quadratic), norm, materialAmbient, materialSpecular);
  }

  return vec3(0.0);
}

void main() {
  vec3 norm = normalize(Normal);
  vec3 materialAmbient = vec3(texture(material.diffuse, TexCoords));
  vec3 materialSpecular = vec3(texture(material.specular, TexCoords));

  vec3 res = vec3(0.0);
  res += calcDirLight(dirLight, norm, materialAmbient, materialSpecular);
  for(int i = 0; i < POINT_LIGHTS_COUNT; i++) {
    res += calcPointLight(pointLights[i], norm, materialAmbient, materialSpecular);
  }
  res += calcSpotLight(spotLight, norm, materialAmbient, materialSpecular);

  FragColor = vec4(res, 1.0);
}
