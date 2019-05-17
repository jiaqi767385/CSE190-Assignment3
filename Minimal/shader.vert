#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 vertexUV;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec2 UV;

void main()
{
    gl_Position = projection * model * view * vec4(position.x, position.y, position.z, 1.0);
	UV = vertexUV;
}
