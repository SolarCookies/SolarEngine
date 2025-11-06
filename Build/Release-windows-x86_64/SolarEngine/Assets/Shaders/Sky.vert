#version 330 core

layout (location = 0) in vec3 apos;

out vec3 viewDir;

uniform mat4 camMatrix;
uniform mat4 model;
uniform vec3 cameraPos;

void main()
{
	vec4 worldPos = model * vec4(apos * 50.0, 1.0f);
	gl_Position = camMatrix * worldPos;
	viewDir = normalize(worldPos.xyz - cameraPos);
}