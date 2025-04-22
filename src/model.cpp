#include "model.hpp"
#include "pskreader.hpp"

#include <limits>
#include <algorithm>

#include <iostream>
#include <optional>

std::optional<GLuint> MESH_VAO;

struct CompleteVertex
{
    // Position coordinates
    float x;
    float y;
    float z;

    // Texture coordinates
    float u;
    float v;
    int32_t materialIndex;
};

GLuint* getIndicesArray(std::vector<PSK_Face> &faces)
{
    GLuint *array = (GLuint*) malloc(faces.size() * 3 * sizeof(GLuint));

    size_t index = 0;
    for (PSK_Face &face : faces)
    {
        array[index] = face.wedge0;
        index++;

        array[index] = face.wedge1;
        index++;

        array[index] = face.wedge2;
        index++;
    } 

    return array;
}

CompleteVertex* getVertexArray(std::vector<PSK_Point> &points, std::vector<PSK_Wedge> &wedges)
{
    CompleteVertex *array = (CompleteVertex*) malloc(wedges.size() * sizeof(CompleteVertex));
    for(size_t i = 0; i < wedges.size(); i++)
    {
        PSK_Wedge *wedge = &wedges[i];
        PSK_Point *point = &points[wedge->pointIndex];

        array[i].x = point->x;
        array[i].y = point->y;
        array[i].z = point->z;

        array[i].u = wedge->u;
        array[i].v = wedge->v;
        array[i].materialIndex = wedge->materialIndex;
    }

    return array;
}

// Singleton pattern for vertex array object
void UseMeshVAO()
{
    if(MESH_VAO.has_value())
    {
        glBindVertexArray(MESH_VAO.value());
        return;
    }

    GLuint arr;
    glGenVertexArrays(1, &arr);
    glBindVertexArray(arr);
    
    MESH_VAO = arr;
}

Mesh::Mesh(PSK_MeshData &d)
{
    GLenum err;

    glGenVertexArrays(1, &buffers.vao);
    glBindVertexArray(buffers.vao);

    glGenBuffers(1, &buffers.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.vertexBuffer);

    CompleteVertex *vertices = getVertexArray(d.points, d.wedges);
    glBufferData(GL_ARRAY_BUFFER, d.wedges.size() * sizeof(CompleteVertex), vertices, GL_STATIC_DRAW);
    free(vertices);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CompleteVertex), (void*)offsetof(CompleteVertex, x));
    glEnableVertexAttribArray(0);

    // u,v coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CompleteVertex), (void*)offsetof(CompleteVertex, u));
    glEnableVertexAttribArray(1);

    // material index
    glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, sizeof(CompleteVertex), (void*)offsetof(CompleteVertex, materialIndex));
    glEnableVertexAttribArray(2);

    
    glGenBuffers(1, &buffers.faceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.faceBuffer);
    
    GLuint *indices = getIndicesArray(d.faces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, d.faces.size() * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);
    triangleCount = d.faces.size() * 3;
    free(indices);
    
    minPoint.x = std::numeric_limits<float>::max();
    minPoint.y = std::numeric_limits<float>::max();
    minPoint.z = std::numeric_limits<float>::max();

    maxPoint = glm::vec3(std::numeric_limits<float>::min());

    for (const PSK_Point &point: d.points)
    {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        minPoint.z = std::min(minPoint.z, point.z);


        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
        maxPoint.z = std::max(maxPoint.z, point.z);
    }
}

void Mesh::Draw()
{
    glBindVertexArray(buffers.vao);
    glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, (void*)(0));
}

Model::Model()
{
    LoadMeshFromFile("assets/Game/Character/Item/Meshes/hwo/Face/hwo_fac/Meshes/SK_CH_hwo_fac.psk") ;
    LoadMeshFromFile("assets/Game/Character/Item/Meshes/hwo/Hair/hwo_har_1p/Meshes/SK_CH_hwo_har_1p.psk") ;
    LoadMeshFromFile("assets/Game/Character/Item/Meshes/hwo/Lower/hwo_bdl_taekwondo/Meshes/SK_CH_hwo_bdl_taekwondo.psk");
    LoadMeshFromFile("assets/Game/Character/Item/Meshes/hwo/Upper/hwo_bdu_1p/Meshes/SK_CH_hwo_bdu_1p.psk") ;
}

void Model::LoadMeshFromFile(const std::string &pskFile)
{
    PSK_MeshData *mData = ReadPSKFile(pskFile);
    Mesh *mesh = new Mesh(*mData);
    meshes.push_back(mesh);
    delete mData;

    std::cout << "Mesh count: " << meshes.size();
}

Model::~Model()
{
    for (Mesh *mesh : meshes)
    {
        delete mesh;
    }
}

void Model::Draw(ShaderProgram &shader, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection)
{
    shader.use();
    shader.setMat4("modelMatrix", model);
    shader.setMat4("projectionMatrix", projection);
    shader.setMat4("viewMatrix", view);
    int i = 0;
    for (Mesh *mesh : meshes)
    {
        mesh->Draw();
        i++;
    }

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}