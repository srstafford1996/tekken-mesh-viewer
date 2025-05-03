#include <cstring> // for memcpy

#include <map>
#include <fstream>
#include <iostream>
#include <filesystem>


#include "../../Common/util.hpp"
#include "mesh.hpp"

using namespace uam;

struct CompleteVertex
{
    float x;
    float y;
    float z;

    float u;
    float v;
    int32_t materialIndex;
};

PSK_MeshData* loadPSK(const std::string &pskPath);
CompleteVertex *getVertexArray(std::vector<PSK_Point> &points, std::vector<PSK_Wedge> &wedges);
std::map<std::string, std::string> readKeyValueFile(const std::string &filePath);

PSK_ChunkHeader readChunkHeader(std::ifstream &file);
std::vector<PSK_Point> readPointsChunk(std::ifstream &pskFile, int32_t dataSize, int32_t dataCount);
std::vector<PSK_Wedge> readWedgesChunk(std::ifstream &pskFile, int32_t dataSize, int32_t dataCount);
std::vector<PSK_Face> readFacesChunk(std::ifstream &pskFile, std::string headerId, int32_t dataSize, int32_t dataCount);
std::vector<PSK_Material> readMaterialsChunk(std::ifstream &pskFile, std::string headerId, int32_t dataSize, int32_t dataCount);

/***************** MESH ASSET IMPLEMENTATION ******************/
MeshAsset::MeshAsset(std::string &pskPath)
{
    this->pskPath = pskPath;
}

MeshAsset::~MeshAsset()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    for (Material* material : materials)
    {
        delete material;
    }
}

void MeshAsset::LoadData()
{
    PSK_MeshData *data = loadPSK(pskPath);

    // Generate buffers
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Load vertex and face data
    CompleteVertex *vertices = getVertexArray(data->points, data->wedges);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CompleteVertex) * data->wedges.size(), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CompleteVertex), (void*)offsetof(CompleteVertex, x));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CompleteVertex), (void*)offsetof(CompleteVertex, u));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, sizeof(CompleteVertex), (void*)offsetof(CompleteVertex, materialIndex));
    glEnableVertexAttribArray(2);
    
    delete[] vertices;

    GLuint *indices = buildIndicesArray(data->faces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * data->faces.size() * 3, indices, GL_STATIC_DRAW);
    delete[] indices;

    std::filesystem::path directory = std::filesystem::path(pskPath).remove_filename().generic_string();
    std::string stem = std::filesystem::path(pskPath).stem().generic_string();

    // Load map file
    std::map<std::string, std::string> keyMap = readKeyValueFile( std::filesystem::path(pskPath).replace_extension(".skmap").generic_string() );

    // Load materials
    for (const auto &materialData : data->materials)
    {
        std::filesystem::path materialPath = keyMap[materialData.name];
        std::map<std::string, std::string> materialKeyMap = readKeyValueFile(materialPath.generic_string());

        Material *material = new Material(materialKeyMap, keyMap);
        materials.push_back(material);
    }

    // Just to prevent any mistaken additional writes to this VAO
    glBindVertexArray(0);
}

void MeshAsset::Draw(ShaderProgram &shader)
{
    glBindVertexArray(VAO);

    // For each material batch
    // bind the appropriate material
    // and render

    uint64_t countOffset = 0;
    for (size_t i = 0; i < materialBatchSizes.size(); i++)
    {
        // Bind the main texture array (diffuse, normal, spec)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, materials[i]->mainTexArray);

        // And any extras
        for (size_t k = 0; k < materials[i]->otherTextures.size(); k++)
        {
            glActiveTexture(GL_TEXTURE1 + k);
            glBindTexture(GL_TEXTURE_2D, materials[i]->otherTextures[k]);
        }

        shader.setInt("otherTexturesSize", materials[i]->otherTextures.size());
        glDrawElements(GL_TRIANGLES, materialBatchSizes[i], GL_UNSIGNED_INT, (void*)(countOffset * sizeof(GLuint)));
        countOffset += materialBatchSizes[i];
    }
}

GLuint* MeshAsset::buildIndicesArray(std::vector<PSK_Face> &faces)
{

    GLuint *array = new GLuint[3 * faces.size()];

    // For batch rendering
    // assuming all faces are sorted by material indices
    materialBatchSizes.push_back(0);
    
    int8_t currMatIndex = faces[0].materialIndex;
    for (size_t i = 0; i < faces.size(); i++)
    {
        array[i * 3] = faces[i].wedge0;
        array[i * 3 + 1] = faces[i].wedge1;
        array[i * 3 + 2] = faces[i].wedge2;

        // Increment count of materal batch
        // or move to the next one
        if (currMatIndex == faces[i].materialIndex)
        {
            materialBatchSizes[faces[i].materialIndex] += 3;
            continue;
        }

        currMatIndex = faces[i].materialIndex;
        materialBatchSizes.push_back(3);
    }
    return array;
}

/*************************** UTIL FUNCTIONS ***************************/

CompleteVertex *getVertexArray(std::vector<PSK_Point> &points, std::vector<PSK_Wedge> &wedges)
{
    CompleteVertex *array = new CompleteVertex[wedges.size()];

    // Should be 0 as we expect
    // the array to be sorted by materialIndex
    int32_t currMatIndex = wedges[0].materialIndex;
    for (size_t i = 0; i < wedges.size(); i++)
    {
        array[i].x = points[wedges[i].pointIndex].x;
        array[i].y = points[wedges[i].pointIndex].y;
        array[i].z = points[wedges[i].pointIndex].z;

        array[i].u = wedges[i].u;
        array[i].v = wedges[i].v;
        array[i].materialIndex = wedges[i].materialIndex;
    }
    return array;
}


std::map<std::string, std::string> readKeyValueFile(const std::string &filePath)
{
    // KeyValue as in key=value\n files
    // This will be used for .mat and .skmap files
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    size_t filesize = (size_t) file.tellg();
    file.seekg(0);
     
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filePath << "\n";
        throw std::runtime_error("failed to open key value file");
    }

    std::map<std::string, std::string> kvMap;

    char buffer;
    while (file.tellg() < filesize)
    {
        std::string key, value;

        // Read key
        while (file.tellg() < filesize)
        {
            file.read(&buffer, 1);
            if (buffer == '=') break;

            key += buffer;
        }

        while (file.tellg() < filesize)
        {
            file.read(&buffer, 1);
            if (buffer == '\n') break;

            value += buffer;
        }

        // fucking windows line enders
        rtrim(value);

        kvMap[key] = value;
    }

    file.close();

    return kvMap;
}


PSK_ChunkHeader readChunkHeader(std::ifstream &file)
{
    PSK_ChunkHeader header;

    // First 20 bytes header
    header.chunkId.resize(20);
    file.read(header.chunkId.data(), 20);
    
    // 4 bytes type flag
    // 4 bytes data size
    // 4 bytes data count
    // memcpy to avoid undefined behavior

    char intBuffer[4];

    file.read(intBuffer, 4);
    std::memcpy(&header.typeFlag, intBuffer, 4);


    file.read(intBuffer, 4);
    std::memcpy(&header.dataSize, intBuffer, 4);

    file.read(intBuffer, 4);
    std::memcpy(&header.dataCount, intBuffer, 4);

    std::cout << "Header: " << header.chunkId << " | Type Flag: " << header.typeFlag << " | Data Size: "
        << header.dataSize << " | Data Count: " << header.dataCount << std::endl;

    return header;

}

std::vector<PSK_Point> readPointsChunk(std::ifstream &pskFile, int32_t dataSize, int32_t dataCount)
{
    std::vector<PSK_Point> points(dataCount);

    char buffer[4];

    int64_t bytesRead = 0;
    for (PSK_Point &point : points)
    {
        pskFile.read(buffer, 4);
        std::memcpy(&point.x, buffer, 4);
        

        pskFile.read(buffer, 4);
        std::memcpy(&point.z, buffer, 4);
        
        pskFile.read(buffer, 4);
        std::memcpy(&point.y, buffer, 4);
        
        bytesRead += 12;
    }

    if (bytesRead != (dataSize * dataCount))
    {
        std::cout << "Warning: incorrect number of bytes read from points chunk" << std::endl;
    }

    return points;
}

std::vector<PSK_Wedge> readWedgesChunk(std::ifstream &pskFile, int32_t dataSize, int32_t dataCount)
{

    std::vector<PSK_Wedge> wedges(dataCount);

    char buffer[4];
    int64_t bytesRead = 0;
    for (PSK_Wedge &wedge : wedges)
    {
        pskFile.read(buffer, 4);
        std::memcpy(&wedge.pointIndex, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&wedge.u, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&wedge.v, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&wedge.materialIndex, buffer, 4);

        bytesRead += 16;
    }

    if (bytesRead != (dataSize * dataCount))
    {
        std::cout << "Warning: incorrect number of bytes read from wedges chunk(Expected: " << dataSize * dataCount << " | Read: "<< bytesRead << ")" << std::endl;
    }

    return wedges;

}

std::vector<PSK_Face> readFacesChunk(std::ifstream &pskFile, std::string headerId, int32_t dataSize, int32_t dataCount)
{

    std::vector<PSK_Face> faces(dataCount);

    char buffer[4] = {0, 0, 0, 0};
    int64_t bytesRead = 0;

    if (headerId == "FACE0000")
    {
        for (PSK_Face &face : faces)
        {
            pskFile.read(buffer, 2); 
            std::memcpy(&face.wedge0, buffer, 4);            

            pskFile.read(buffer, 2); 
            std::memcpy(&face.wedge1, buffer, 4);            

            pskFile.read(buffer, 2); 
            std::memcpy(&face.wedge2, buffer, 4);

            buffer[1] = 0;
            pskFile.read(buffer, 1);
            std::memcpy(&face.materialIndex, buffer, 1);

            pskFile.read(buffer, 1);
            std::memcpy(&face.auxMaterialIndex, buffer, 1);

            pskFile.read(buffer, 4);
            std::memcpy(&face.smoothingGroups, buffer, 4);


            bytesRead += 12;
        }
    }
    else if (headerId == "FACE0032")
    {
        std::cout << "Umodel face chunk detected" << std::endl;
        for (PSK_Face &face : faces)
        {
            pskFile.read(buffer, 4); 
            std::memcpy(&face.wedge0, buffer, 4);            

            pskFile.read(buffer, 4); 
            std::memcpy(&face.wedge1, buffer, 4);            

            pskFile.read(buffer, 4); 
            std::memcpy(&face.wedge2, buffer, 4);

            buffer[1] = 0;
            buffer[2] = 0;
            buffer[3] = 0;

            pskFile.read(buffer, 1);
            std::memcpy(&face.materialIndex, buffer, 1);

            pskFile.read(buffer, 1);
            std::memcpy(&face.auxMaterialIndex, buffer, 1);

            pskFile.read(buffer, 4);
            std::memcpy(&face.smoothingGroups, buffer, 4);
            bytesRead += 18;
        }
    }

    if (bytesRead != (dataSize * dataCount))
    {
        std::cout << "Warning: incorrect number of bytes read from faces chunk(Expected: " << dataSize * dataCount << " | Read: "<< bytesRead << ")" << std::endl;
    }

    return faces;

}

std::vector<PSK_Material> readMaterialsChunk(std::ifstream &pskFile, std::string headerId, int32_t dataSize, int32_t dataCount)
{
    std::vector<PSK_Material> materials(dataCount);

    char buffer[4];
    for (PSK_Material &material : materials)
    {
        material.name.resize(64);
        pskFile.read(material.name.data(), 64);
        rtrim(material.name);

        pskFile.read(buffer, 4);
        std::memcpy(&material.textureIndex, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.polyFlags, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.auxMaterial, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.auxFlags, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.lodBias, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.lodStyle, buffer, 4);

        std::cout << "Name: " << material.name << " | Texture Index: " << material.textureIndex
            << " | Poly Flags: " << material.polyFlags << " | Aux Material: " << material.auxMaterial
            << " | Aux Flags: " << material.auxFlags << " | " << material.lodBias << " | " << material.lodStyle << "\n\n";
    }

    return materials;
}

PSK_MeshData* loadPSK(const std::string &pskPath)
{
    std::ifstream pskFile(pskPath, std::ios::ate | std::ios::binary);
    
    if (!pskFile.is_open())
    {
        std::cout << "Failed to open file: " << pskPath << "\n";
        throw std::runtime_error("failed to open psk file");
    }
    
    size_t filesize = (size_t) pskFile.tellg();
    pskFile.seekg(0);
    std::cout << "File \"" << pskPath << "\" loaded (" << filesize / 1000 << " KB)\n";
    
    
    PSK_MeshData *data = new PSK_MeshData;
    
    // Load data
    while (pskFile.tellg() != filesize)
    {
        PSK_ChunkHeader header = readChunkHeader(pskFile);
        std::string id = header.chunkId.substr(0, 8);
        if (id == "PNTS0000")
        {
            data->points = readPointsChunk(pskFile, header.dataSize, header.dataCount);
            continue;
        };

        if (id == "VTXW0000")
        {
            data->wedges = readWedgesChunk(pskFile, header.dataSize, header.dataCount);
            continue;
        }

        if (id == "FACE0000" || id == "FACE3200")
        {
            data->faces = readFacesChunk(pskFile, id, header.dataSize, header.dataCount);
            continue;
        }

        if (id == "MATT0000")
        {
            data->materials = readMaterialsChunk(pskFile, id, header.dataSize, header.dataCount);
            continue;
        }

        pskFile.seekg( (size_t) pskFile.tellg() + (header.dataCount * header.dataSize) );
    }

    pskFile.close();

    return data;
}
