#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixValue;

void main() {
  FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(TexCoord.s, 1.0 - TexCoord.t)), mixValue);
}
