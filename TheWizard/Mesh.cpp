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

    std::cout << std::endl << &MPC << " - MPC VAL";

}


glm::vec4 Mesh::getPushConstantsData()
{
    return pushConstants.data;
}
glm::mat4 Mesh::getPushConstantsMatrix()
{
    return pushConstants.render_matrix;
}

Mesh::Mesh(const char* filename)
{


}


void Mesh::meshInit(VkPhysicalDevice PHYSICAL_DEVICE_P, VkDevice LOGICAL_DEVICE_P, VkInstance INSTACE_P, VkCommandPool COMMAND_POOL_P, VkCommandBuffer COMMAND_BUFFER_P, VkQueue GRAPHICS_QUEUE_P,VmaAllocator allocator, VmaAllocatorCreateInfo ALLOCATOR_INFO)
{

    ALLOCATOR = allocator;
    VmaAllocatorInfo = ALLOCATOR_INFO;

    PHYSICAL_DEVICE = PHYSICAL_DEVICE_P;
    LOGICAL_DEVICE = LOGICAL_DEVICE_P;
    INSTANCE = INSTACE_P;
    GRAPHICS_QUEUE = GRAPHICS_QUEUE_P;
    COMMAND_POOL = COMMAND_POOL_P;
    COMMAND_BUFFER = COMMAND_BUFFER_P;

    const char * filename = "assets/bugatti.obj";
    const char* mtl_dir = "assets/";


    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, mtl_dir);
    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << err << std::endl;
    if (!ret) exit(1);


    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex v = {};
            if (index.vertex_index >= 0 && index.vertex_index < attrib.vertices.size() / 3)
            {
                v.position = glm::vec3(attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]);
            }
            if (index.normal_index >= 0 && index.normal_index < attrib.normals.size() / 3)
            {
                v.normal = glm::vec3(attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]);
            }
            if (index.texcoord_index >= 0 && index.texcoord_index < attrib.texcoords.size() / 2)
            {
                v.texcoord = glm::vec2(attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]); // flip texcoord y-axis to match OpenGL convention
            }
            vertices.push_back(v);
        }
    }


    std::cout << "Loaded " << filename << " with " << vertices.size() << " vertices" << std::endl;

    for (int i = 0;i<vertices.size();i++)
    {
        if (i % 2 == 0)
        {
            vertices[i].color = glm::vec3(1.0f, 0.0f, 0.0f);
        }
      
    }

    uploadMesh();
}


uint32_t Mesh::getVerticesSize()
{
    //std::cout << std::endl << "IN MESH :" << vertices.size();
    return vertices.size();
}

VkBuffer* Mesh::getVertexBuffer()
{
    return &VERTEX_BUFFER.BUFFER;
}


void Mesh::loadMesh()
{
    vertices.resize(8);

   //vertices = {
   //   // front
   //   { { -0.5f, -0.5f,  0.5f }, { 0.f, 0.f, 1.f } },
   //   { {  0.5f, -0.5f,  0.5f }, { 0.f, 0.f, 1.f } },
   //   { {  0.5f,  0.5f,  0.5f }, { 0.f, 0.f, 1.f } },
   //   { { -0.5f,  0.5f,  0.5f }, { 0.f, 0.f, 1.f } },
   //
   //   // back
   //   { { -0.5f, -0.5f, -0.5f }, { 0.f, 0.f, -1.f } },
   //   { { -0.5f,  0.5f, -0.5f }, { 0.f, 0.f, -1.f } },
   //   { {  0.5f,  0.5f, -0.5f }, { 0.f, 0.f, -1.f } },
   //   { {  0.5f, -0.5f, -0.5f }, { 0.f, 0.f, -1.f } },
   //
   //   // top
   //   { { -0.5f,  0.5f,  0.5f }, { 0.f, 1.f, 0.f } },
   //   { {  0.5f,  0.5f,  0.5f }, { 0.f, 1.f, 0.f } },
   //   { {  0.5f,  0.5f, -0.5f }, { 0.f, 1.f, 0.f } },
   //   { { -0.5f,  0.5f, -0.5f }, { 0.f, 1.f, 0.f } },
   //
   //   // bottom
   //   { { -0.5f, -0.5f,  0.5f }, { 0.f, -1.f, 0.f } },
   //   { { -0.5f, -0.5f, -0.5f }, { 0.f, -1.f, 0.f } },
   //   { {  0.5f, -0.5f, -0.5f }, { 0.f, -1.f, 0.f } },
   //   { {  0.5f, -0.5f,  0.5f }, { 0.f, -1.f, 0.f } },
   //
   //   // left
   //   { { -0.5f, -0.5f,  0.5f }, { -1.f, 0.f, 0.f } },
   //   { { -0.5f,  0.5f,  0.5f }, { -1.f, 0.f, 0.f } },
   //   { { -0.5f,  0.5f, -0.5f }, { -1.f, 0.f, 0.f } },
   //   { { -0.5f, -0.5f, -0.5f }, { -1.f, 0.f, 0.f } },
   //
   //   // right
   //   { {  0.5f, -0.5f,  0.5f }, { 1.f, 0.f, 0.f } },
   //   { {  0.5f, -0.5f, -0.5f }, { 1.f, 0.f, 0.f } },
   //   { {  0.5f,  0.5f, -0.5f }, { 1.f, 0.f, 0.f } },
   //   { {  0.5f,  0.5f,  0.5f }, { 1.f, 0.f, 0.f } },
   //;


    //loop over vertices
    
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
