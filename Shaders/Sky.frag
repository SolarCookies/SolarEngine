#version 330 core

out vec4 FragColor;

in vec3 color;

in vec2 texCoord;

in vec3 Normal;
in vec3 crntPos;

uniform sampler2D tex0; // DiffuseMap

uniform sampler2D tex1; // NormalMap

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform int unlit; // Unlit flag

void main()
{
    vec4 texColor = texture(tex0, texCoord * vec2(1.0f, -1.0f)); // Flip Y for correct texture sampling
    FragColor = texColor; // Use the color input directly
}