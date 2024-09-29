#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
  // ambient
  float ambient = 0.1;

  // diffuse
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diffuse = max(dot(norm, lightDir), 0.0);

  // specular
  float specularStrength = 1.0;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float specularBase = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  float specular = specularStrength * specularBase;

  FragColor = vec4((ambient + diffuse + specular) * lightColor * objectColor, 1.0);
}
