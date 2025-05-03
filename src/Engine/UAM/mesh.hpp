#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>
#include <stdint.h>

#include "types.hpp"
#include "material.hpp"
#include "../shader.hpp"

namespace uam
{
    class MeshAsset
    {
        std::string pskPath;
        std::vector<Material *> materials;
        std::vector<uint32_t> materialBatchSizes;

        GLuint VAO;
        GLuint VBO;
        GLuint EBO;

        GLuint *buildIndicesArray(std::vector<PSK_Face> &faces);

    public:
        MeshAsset(std::string &pskPath);
        ~MeshAsset();

        void LoadData();
        void Draw(ShaderProgram &shader);
    };
}