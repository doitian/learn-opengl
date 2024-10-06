#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

uniform Material material;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
  // diffuse
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  vec3 diffuse = max(dot(norm, lightDir), 0.0) * material.diffuse;

  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * material.specular;

  FragColor = vec4((material.ambient + diffuse + specular) * lightColor, 1.0);
}
