#include "Mesh.h"


Mesh::Mesh()
{
    ////we will have just 1 vertex buffer binding, with a per-vertex rate
    VkVertexInputBindingDescription mainBinding = {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    description.BINDINGS.push_back(mainBinding);
    
    //Position will be stored at Location 0
    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);
    
    //Normal will be stored at Location 1
    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(Vertex, normal);
    
    //Color will be stored at Location 2
    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.binding = 0;
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(Vertex, color);
    
    description.ATTRIBUTES.push_back(positionAttribute);
    description.ATTRIBUTES.push_back(normalAttribute);
    description.ATTRIBUTES.push_back(colorAttribute);

}


Mesh::Mesh(const char* filename)
{

    struct IndexHash {
        std::size_t operator()(const tinyobj::index_t& idx) const {
            std::size_t h1 = std::hash<int>()(idx.vertex_index);
            std::size_t h2 = std::hash<int>()(idx.normal_index);
            std::size_t h3 = std::hash<int>()(idx.texcoord_index);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    // Define a custom equality operator for tinyobj::index_t objects
    struct IndexEqual {
        bool operator()(const tinyobj::index_t& lhs, const tinyobj::index_t& rhs) const {
            return lhs.vertex_index == rhs.vertex_index &&
                lhs.normal_index == rhs.normal_index &&
                lhs.texcoord_index == rhs.texcoord_index;
        }
    };


    //attrib will contain the vertex arrays of the file
    tinyobj::attrib_t attrib;
    //shapes contains the info for each separate object in the file
    std::vector<tinyobj::shape_t> shapes;
    //materials contains the information about the material of each shape, but we won't use it.
    std::vector<tinyobj::material_t> materials;

    std::unordered_map<tinyobj::index_t, size_t, IndexHash, IndexEqual> vertex_map;


    //error and warning output from the load function
    std::string warn;
    std::string err;

    //load the OBJ file
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
    //make sure to output the warnings to the console, in case there are issues with the file
    if (!warn.empty()) 
    {
        std::cout << "WARN: " << warn << std::endl;
    }
    //if we have any error, print it to the console, and break the mesh loading.
    //This happens if the file can't be found or is malformed
    if (!err.empty())
    {
        std::cerr << err << std::endl;
    }
    bool valid = false;

    std::cout << std::endl << vertices.size() << std::endl << attrib.vertices.size();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {

            // Determine the number of vertices in the face
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face
            for (size_t v = 0; v < fv; v++) {
                // Access the index of the current vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                // Check if the vertex has already been loaded
                auto it = vertex_map.find(idx);
                if (it != vertex_map.end())
                {
                    // Reuse the existing vertex
                    vertices.push_back(vertices[it->second]);
                }
                else
                {
                    // Create a new vertex
                    Vertex new_vert;

                    // Set the position of the vertex
                    new_vert.position.x = attrib.vertices[3 * idx.vertex_index + 0];
                    new_vert.position.y = attrib.vertices[3 * idx.vertex_index + 1];
                    new_vert.position.z = attrib.vertices[3 * idx.vertex_index + 2];

                    // Set the normal of the vertex
                    if (idx.normal_index >= 0) {
                        new_vert.normal.x = attrib.normals[3 * idx.normal_index + 0];
                        new_vert.normal.y = attrib.normals[3 * idx.normal_index + 1];
                        new_vert.normal.z = attrib.normals[3 * idx.normal_index + 2];
                    }

                    // Set the texture coordinates of the vertex
                    if (idx.texcoord_index >= 0) {
                        new_vert.texcoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                        new_vert.texcoord.y = attrib.texcoords[2 * idx.texcoord_index + 1];
                    }

                    // Set the color of the vertex as the normal (for display purposes)
                   // new_vert.color.x = new_vert.normal.x;
                    //new_vert.color.y = new_vert.normal.y;
                   // new_vert.color.z = new_vert.normal.z;

                    // Add the vertex to the mesh's vertex list
                    vertices.push_back(new_vert);

                    // Add the vertex index to the map
                    vertex_map[idx] = vertices.size() - 1;
                }
            }

            // Update the index offset for the next face
            index_offset += fv;
        }
    }

   // std::cout << std::endl << vertices.size() << std::endl << attrib.vertices.size();

}


void Mesh::meshInit(VkPhysicalDevice PHYSICAL_DEVICE_P, VkDevice LOGICAL_DEVICE_P, VkInstance INSTACE_P, VkCommandPool COMMAND_POOL_P, VkCommandBuffer COMMAND_BUFFER_P, VkQueue GRAPHICS_QUEUE_P)
{
    PHYSICAL_DEVICE = PHYSICAL_DEVICE_P;
    LOGICAL_DEVICE = LOGICAL_DEVICE_P;
    INSTANCE = INSTACE_P;
    GRAPHICS_QUEUE = GRAPHICS_QUEUE_P;
    COMMAND_POOL = COMMAND_POOL_P;
    COMMAND_BUFFER = COMMAND_BUFFER_P;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = PHYSICAL_DEVICE;
    allocatorInfo.device = LOGICAL_DEVICE;
    allocatorInfo.instance = INSTANCE;
    vmaCreateAllocator(&allocatorInfo, &ALLOCATOR);
  
    loadMesh();
}


uint32_t Mesh::getVerticesSize()
{
    return vertices.size();
}

VkBuffer* Mesh::getVertexBuffer()
{
    return &VERTEX_BUFFER.BUFFER;
}


void Mesh::loadMesh()
{
    
    //make the array 3 vertices long
    vertices.resize(3);

    //vertex positions
    vertices[0].position = { 0.5f, 1.f, 0.0f };
    vertices[1].position = { -1.f, 1.f, 0.0f };
    vertices[2].position = { 0.f,-1.f, 0.0f };

    //vertex colors, all green
    vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
    vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
    vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green
    
    uploadMesh();
}

void Mesh::uploadMesh()
{
    //allocate vertex buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //this is the total size, in bytes, of the buffer we are allocating
    bufferInfo.size = vertices.size() * sizeof(Vertex);
    //this buffer is going to be used as a Vertex Buffer
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    //let the VMA library know that this data should be writeable by CPU, but also readable by GPU
     VmaAllocationCreateInfo vmaallocInfo = {};
     vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    //allocate the buffer  &mesh._vertexBuffer._allocation
    vmaCreateBuffer(ALLOCATOR, &bufferInfo, &vmaallocInfo, &VERTEX_BUFFER.BUFFER, &VERTEX_BUFFER.ALLOCATION, nullptr);

    //   _mainDeletionQueue.push_function([=]() {
    //
    //       vmaDestroyBuffer(ALLOCATOR, VERTEX_BUFFER.BUFFER, VERTEX_BUFFER.ALLOCATION);
    //       });

    void* data;

    std::cout << std::endl << vertices.data() << " " << vertices.size();

    std::cout << std::endl << ALLOCATOR << " " << &bufferInfo << " " << &vmaallocInfo << " " << &VERTEX_BUFFER.BUFFER << " " << &VERTEX_BUFFER.ALLOCATION << " ";

    vmaMapMemory(ALLOCATOR, VERTEX_BUFFER.ALLOCATION, &data);

    memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(ALLOCATOR, VERTEX_BUFFER.ALLOCATION);

}
