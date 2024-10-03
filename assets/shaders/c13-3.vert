#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 LightingColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform mat3 transposedInverseModel;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  vec3 fragPos = vec3(model * vec4(aPos, 1.0));
  vec3 normal = transposedInverseModel * aNormal;

  // ambient
  float ambient = 0.1;

  // diffuse
  vec3 norm = normalize(normal);
  vec3 lightDir = normalize(lightPos - fragPos);
  float diffuse = max(dot(norm, lightDir), 0.0);

  // specular
  float specularStrength = 1.0;
  vec3 viewDir = normalize(viewPos - fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float specularBase = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  float specular = specularStrength * specularBase;

  LightingColor = (ambient + diffuse + specular) * lightColor;
}
