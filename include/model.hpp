#include <GL/glew.h>
#include <vector>
#include <string>

#include "pskreader.hpp"
#include "shader.hpp"

struct Buffers
{
    GLuint vertexBuffer;
    GLuint faceBuffer;
    GLuint vao;
};

class Mesh
{
public:
    uint32_t triangleCount;
    ShaderProgram *shader;

    glm::vec3 minPoint;
    glm::vec3 maxPoint;

    Buffers buffers;
    
    Mesh(PSK_MeshData &data);
    void Draw();
};

class Model
{
public:
    uint32_t triangles;

    Buffers buffers;
    std::vector<Mesh*> meshes;

    Model();
    ~Model();
    void Draw(ShaderProgram &shader, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
    void LoadMeshFromFile(const std::string &pskFile);
};
