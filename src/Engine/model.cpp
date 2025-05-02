#include <glm.hpp>
#include <string>
#include <iostream>

#include "shader.hpp"
#include "../UAM/uam.hpp"

#include "model.hpp"

Model::Model()
{
    // Transformation matrix
    // Down the line can be used for movement/rotation etc
    modelMatrix = glm::mat4(1.0);
}

Model::~Model()
{
    for (uam::MeshAsset *mesh : meshes)
    {
        delete mesh;
    }
}

void Model::Draw(ShaderProgram &shader)
{
    // Assume caller has binded the matrix program
    // and set view and projection matrix

    shader.setMat4("modelMatrix", modelMatrix);
    for (uam::MeshAsset *mesh : meshes)
    {
        mesh->Draw(shader);
    }
}

void Model::AddMesh(std::string pskPath)
{
    uam::MeshAsset *mesh = new uam::MeshAsset(pskPath); 
    mesh->LoadData();

    meshes.push_back(mesh);
    return;
}