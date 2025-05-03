#version 430 core

uniform sampler2DArray mainTextureArray;
uniform sampler2D otherTextures[16];
uniform int otherTexturesSize;

out vec4 FragColor;
in vec2 oTexCoord;

void main()
{
    // mainTextureArray consists of:
    // diffuse, normal, specpower
    // in that order
    vec4 color = texture(mainTextureArray, vec3(oTexCoord, 0));
    
    // for now we will just use diffuse
    FragColor = color;
}