#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

#include "textures.hpp"
#include "../Engine/shader.hpp"

namespace uam
{

struct CompleteVertex
{
    float x;
    float y;
    float z;

    float u;
    float v;
    int32_t materialIndex;
};

struct PSK_ChunkHeader
{
    std::string chunkId;
    int32_t typeFlag;
    int32_t dataSize;
    int32_t dataCount;
};

struct PSK_Point
{
    float x;
    float y;
    float z;
};

struct PSK_Wedge
{
    uint32_t pointIndex;
    float u;
    float v;
    int32_t materialIndex;
};

struct PSK_Face
{
    int32_t wedge0;
    int32_t wedge1;
    int32_t wedge2;
    
    int8_t materialIndex;
    int8_t auxMaterialIndex;
    int32_t smoothingGroups;
};

struct PSK_Material
{
    std::string name;

    int32_t textureIndex;
    int32_t polyFlags;
    int32_t auxMaterial;
    int32_t auxFlags;
    int32_t lodBias;
    int32_t lodStyle;
};

struct PSK_MeshData
{
    std::vector<PSK_Point> points;
    std::vector<PSK_Wedge> wedges;
    std::vector<PSK_Face> faces;
    std::vector<PSK_Material> materials;
};


class MeshAsset
{
    std::string pskPath;
    std::vector<Material*> materials;
    std::vector<uint32_t> materialBatchSizes;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    uint32_t indicesCount;
    
    GLuint *buildIndicesArray(std::vector<PSK_Face> &faces);

public:
    MeshAsset(std::string &pskPath);
    ~MeshAsset();

    void LoadData();
    void Draw(ShaderProgram &shader);
};

}