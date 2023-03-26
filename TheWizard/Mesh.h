#pragma once

#ifndef MESH_H
#define MESH_H

#include <vector>
#include "Vulkan.h"
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>

class Vulkan;



class Mesh
{
public:
	Mesh(Vulkan *vulkan);

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;

        static VkVertexInputBindingDescription* getBindingDescription()
        {
            static VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return &bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, position);


            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);


            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
        std::vector<Vertex> vertices;
    };

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	

	uint32_t getVerticesSize();
	VkBuffer *getVertexBuffer();

	VkVertexInputBindingDescription *getBindingDescription();
	array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

	const std::vector<Vertex>& getVertices(size_t start) const { return std::vector<Vertex>(vertices.begin() + start, vertices.end());}
	const std::vector<Vertex>& getVertices() const {return vertices;}

	void loadMesh();
	void uploadMesh();


private:

	Vulkan *VK;

	VkVertexInputBindingDescription bindingDescription;
	array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

	struct Vertex;

	std::vector<Vertex> vertices;

	VmaAllocator ALLOCATOR;
	VmaAllocatorCreateInfo ALLOCATOR_INFO = {};

	struct AllocatedBuffer {
		VkBuffer BUFFER;
		VmaAllocation ALLOCATION;
	};

	AllocatedBuffer VERTEX_BUFFER;


};

#endif // MESH_H