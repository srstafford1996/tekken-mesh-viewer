#pragma once

#include <string>
#include <GL/glew.h>

namespace uam
{
    void UnregisterTexture(const std::string texPath);
    GLuint RegisterTexture(const std::string texPath);
}