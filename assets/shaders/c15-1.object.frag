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
struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
  vec3 materialAmbient = vec3(texture(material.diffuse, TexCoords));
  vec3 materialSpecular = vec3(texture(material.specular, TexCoords));

  // ambient
  vec3 ambient = light.ambient * materialAmbient;

  // diffuse
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - FragPos);
  vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.diffuse * materialAmbient;

  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * light.specular * materialSpecular;

  FragColor = vec4(ambient + diffuse + specular, 1.0);
}
