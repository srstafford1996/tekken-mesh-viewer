#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in int materialIndex;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 oTexCoord;

void main() {

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
    oTexCoord = texCoord;
}