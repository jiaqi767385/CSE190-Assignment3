#version 330 core

// in vec3 Normal;
in vec2 UV;

// You can output many things. The first vec4 type output determines the color of the fragment
out vec3 color;

uniform sampler2D textureShader;

void main()
{
    // Color everything a hot pink color. An alpha of 1.0f means it is not transparent.
    color = texture(textureShader, UV).rgb;
}
