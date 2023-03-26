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

VkCommandBuffer Mesh::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = COMMAND_POOL;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(LOGICAL_DEVICE, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Mesh::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(GRAPHICS_QUEUE);

    vkFreeCommandBuffers(LOGICAL_DEVICE, COMMAND_POOL, 1, &commandBuffer);
}

uint32_t Mesh::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(PHYSICAL_DEVICE, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

}


uint32_t Mesh::getVerticesSize()
{
    //cout << endl << vertices.size() << "v SIZE";
    return vertices.size();
}

void Mesh::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
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

     //cout << endl << ALLOCATOR << " " << &bufferInfo << " " << &vmaallocInfo << " " << &VERTEX_BUFFER.BUFFER << " " << &VERTEX_BUFFER.ALLOCATION << " ";

    //allocate the buffer  &mesh._vertexBuffer._allocation
    vmaCreateBuffer(ALLOCATOR, &bufferInfo, &vmaallocInfo, &VERTEX_BUFFER.BUFFER, &VERTEX_BUFFER.ALLOCATION, nullptr);

    //   _mainDeletionQueue.push_function([=]() {
    //
    //       vmaDestroyBuffer(ALLOCATOR, VERTEX_BUFFER.BUFFER, VERTEX_BUFFER.ALLOCATION);
    //       });

    void* data;

    //cout << endl << vertices.data() << " " << vertices.size();

    vmaMapMemory(ALLOCATOR, VERTEX_BUFFER.ALLOCATION, &data);

    memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(ALLOCATOR, VERTEX_BUFFER.ALLOCATION);

}
