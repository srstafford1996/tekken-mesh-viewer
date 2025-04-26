#pragma once

#include <vector>
#include <string>

#include <glm.hpp>

namespace uam { class MeshAsset; }
class ShaderProgram;

class Model
{

public:
    glm::mat4 modelMatrix;
    std::vector<uam::MeshAsset*> meshes;

    Model();
    ~Model();

    void AddMesh(std::string pskPath);
    void Draw(ShaderProgram &shader);
};