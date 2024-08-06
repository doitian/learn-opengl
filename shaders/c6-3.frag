#version 330 core

in vec3 ourPosition;

out vec4 FragColor;

void main() {
  // Negative rgb components are clamped to 0
  FragColor = vec4(ourPosition, 1.0);
}
