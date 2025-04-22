#include "shader.hpp"

#include "GL/glew.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

ShaderProgram::ShaderProgram(const std::string &vertPath, const std::string &fragPath)
{
    std::ifstream vertFile(vertPath, std::ios::ate | std::ios::binary);
    std::ifstream fragFile(fragPath, std::ios::ate | std::ios::binary);

    if (!vertFile.is_open())
    {
        throw std::runtime_error("failed to open vert shader");
    }

    if (!fragFile.is_open())
    {
        throw std::runtime_error("failed to open frag shader");
    }

    size_t vertSize = (size_t) vertFile.tellg();
    std::vector<char> vertBuffer(vertSize);

    size_t fragSize = (size_t) fragFile.tellg();
    std::vector<char> fragBuffer(fragSize);

    vertFile.seekg(0);
    vertFile.read(vertBuffer.data(), vertSize);
    vertFile.close();

    fragFile.seekg(0);
    fragFile.read(fragBuffer.data(), fragSize);
    fragFile.close();

    const char *vertSource = vertBuffer.data();
    const char *fragSource = fragBuffer.data();

    unsigned int vertShader, fragShader;

    int success;
    char infoLog[512];

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);

    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        std::cerr << "Failed to compile vert shader(" << vertPath << "):"<< infoLog << std::endl;
        throw std::runtime_error("failed to complie vert shader");
    }
    
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cerr << "Failed to compile frag shader(" << fragPath << "):"<< infoLog << std::endl;
        throw std::runtime_error("failed to complie frag shader");
    }

    
    // Shaders compiled
    // Create Program
    programID = glCreateProgram();
    glAttachShader(programID, vertShader);
    glAttachShader(programID, fragShader);
    glLinkProgram(programID);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

void ShaderProgram::use()
{
    glUseProgram(programID);
}

void ShaderProgram::setFloat(const std::string &name, float val)
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), val);
}

void ShaderProgram::setVec3(const std::string &name, const glm::vec3 vec3)
{
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &vec3[0]);
}

void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}
