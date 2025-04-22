#pragma once
#include <string>
#include <glm.hpp>

class ShaderProgram
{
public:
    unsigned int programID;

    ShaderProgram(unsigned int id) : programID(id) {};
    ShaderProgram(const std::string &vertexPath, const std::string  &fragPath);

    void use();

    void setFloat(const std::string &name, float val);
    void setVec3(const std::string &name, const glm::vec3 vec);
    void setMat4(const std::string &name, const glm::mat4 &matrix);
};