#pragma once

#include <string>
#include <map>
#include <GL/glew.h>
#include <vector>

namespace uam
{
    class Material
    {
    public:
        GLuint mainTexArray;
        std::vector<GLuint> otherTextures;

        std::vector<std::string> texPaths;


        Material(std::map<std::string, std::string> &materialData, std::map<std::string, std::string> &keyMap);
        ~Material();
    };

}