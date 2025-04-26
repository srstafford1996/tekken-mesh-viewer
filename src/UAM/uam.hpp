#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

namespace uam
{

class MeshAsset
{
    std::string pskPath;
    std::vector<std::string> texturePaths;
    std::vector<GLuint> textureIDs;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    uint32_t indicesCount;

public:
    MeshAsset(std::string &pskPath);
    ~MeshAsset();

    void LoadData();
    void Draw();
};


}