#version 330 core

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

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;

const int INVERSE_SPECULAR_MAP = 2;

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

void main() {
  vec3 norm = normalize(Normal);
  vec3 materialAmbient = vec3(texture(material.diffuse, TexCoords));
  vec3 materialSpecular = vec3(texture(material.specular, TexCoords));

  vec3 res = vec3(0.0);
  res += calcDirLight(dirLight, norm, materialAmbient, materialSpecular);

  FragColor = vec4(res, 1.0);
}
