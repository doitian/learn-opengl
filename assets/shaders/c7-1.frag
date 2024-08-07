#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
  FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(TexCoord.x, 1.0 - TexCoord.y)), 0.2);
}
