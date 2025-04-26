#include <GL/glew.h>

#include <string>
#include <map>
#include <iostream>

#include "textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "../Common/stb_image.h"


// Texture Registry
struct _texReg
{
    GLuint textureId;
    uint64_t regCount;
};

std::map<std::string, _texReg*> _texRegistry;

GLuint uam::RegisterTexture(const std::string texPath)
{
    if (_texRegistry.count(texPath) > 0 && _texRegistry[texPath] != nullptr && _texRegistry[texPath]->regCount > 0)
    {
        _texRegistry[texPath]->regCount += 1;
        return _texRegistry[texPath]->textureId;
    }

    // If the texture isn't currently registered then do so and return the id
    std::cout << "Registering new texture: " << texPath << std::endl;

    _texReg *newTex = new _texReg;
    _texRegistry[texPath] = newTex;

    newTex->regCount = 1;

    int width, height, channelCount;
    std::cout << "STBI Loading texture.\n";
    unsigned char *data = stbi_load(texPath.c_str(), &width, &height, &channelCount, 0);

    if (!data)
    {
        std::cout << "Failed to load texture: " << texPath << std::endl;
        return 0;
    }

    GLenum format;
    switch (channelCount)
    {
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            std::cout << "Unsupported number of channels(" << channelCount << ")" <<  "in texture file " << texPath;
            stbi_image_free(data);
            UnregisterTexture(texPath);
            return 0;
    }

    glGenTextures(1, &newTex->textureId);
    glBindTexture(GL_TEXTURE_2D, newTex->textureId);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return newTex->textureId;
};

void uam::UnregisterTexture(const std::string texPath)
{
    if (_texRegistry.count(texPath) == 0 || _texRegistry[texPath] == nullptr || _texRegistry[texPath]->regCount == 0)
    {
        std::cout << "Warning! Attempt to unregister texture that already should not exist: " << texPath << std::endl;
        return;
    }

    _texRegistry[texPath]->regCount -= 1;

    if (_texRegistry[texPath] == 0)
    {
        // If the last mesh using this texture wants to unregister
        // delete it from memory

        glDeleteTextures(1, &_texRegistry[texPath]->textureId);
        delete _texRegistry[texPath];
        _texRegistry[texPath] = nullptr;
    }
}