#include <GL/glew.h>

#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "../Common/util.hpp"
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

GLuint registerTexture(const std::string texPath);
void unregisterTexture(const std::string texPath);

GLuint createTextureArray(std::vector<std::string> &texPaths);

uam::Material::Material(std::map<std::string, std::string> &materialData, std::map<std::string, std::string> &keyMap)
{
    // materialData is a map of the [NAME] = [TEXTUREIDENTIFIER] stored in .mat files
    // keyMap is the [IDENTIFIER]=[PATH] stored in .skmap files

    // Here we egister all dependent texture paths
    // and store them for unregistering

    // Diffuse/Normal/SpecPower will be stored in a texture array
    if (materialData.count("Diffuse") != 0)
    {
        texPaths.push_back( keyMap[materialData["Diffuse"]] );
    }

    if (materialData.count("Normal") != 0)
    {
        texPaths.push_back( keyMap[materialData["Normal"]] );
    }

    if (materialData.count("SpecPower") != 0)
    {
        texPaths.push_back( keyMap[materialData["SpecPower"]] );
    }

    mainTexArray = createTextureArray(texPaths);

    for (std::pair<std::string, std::string> dataPair : materialData)
    {
        if (dataPair.first == "Diffuse") continue;
        if (dataPair.first == "Normal") continue;
        if (dataPair.first == "SpecPower") continue;

        texPaths.push_back( keyMap[dataPair.second] );
        otherTextures.push_back( registerTexture(keyMap[dataPair.second]) );
    }
}

uam::Material::~Material()
{
    for (std::string &texPath : texPaths)
    {
        unregisterTexture(texPath);
    }

    glDeleteTextures(1, &mainTexArray);
}

GLuint registerTexture(const std::string texPath)
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
            unregisterTexture(texPath);
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

void unregisterTexture(const std::string texPath)
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


GLuint createTextureArray(std::vector<std::string> &texPaths)
{
    if (!texPaths.size()) return 0;

    int width, height, channelCount;
    std::cout << "STBI Loading initial texture " << texPaths[0] << std::endl;
    unsigned char *data = stbi_load(texPaths[0].c_str(), &width, &height, &channelCount, 4);

    if (!data)
    {
        std::cout << "Failed to load texture: " << texPaths[0] << std::endl;
        return 0;
    }

    std::cout << "Initial texture loaded.\n";

    GLuint arrayID;
    glGenTextures(1, &arrayID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, arrayID);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, texPaths.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);


    for (int i = 1; i < texPaths.size(); i++)
    {
        data = stbi_load(texPaths[i].c_str(), &width, &height, &channelCount, 4);

        if (!data)
        {
            std::cout << "Failed to load texture: " << texPaths[i] << std::endl;
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);



    return arrayID;
}