#pragma once

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <vulkan.h>
#include <SDL_vulkan.h>
#include <vk_mem_alloc.h>



class Mesh
{
public:
	Mesh();

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
	};


	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void meshInit(VkPhysicalDevice PHYSICAL_DEVICE_P, VkDevice LOGICAL_DEVICE_P, VkInstance INSTACE_P, VkCommandPool COMMAND_POOL_P, VkCommandBuffer COMMAND_BUFFER_P, VkQueue GRAPHICS_QUEUE_P);

	uint32_t getVerticesSize();
	VkBuffer *getVertexBuffer();

	//const std::vector<Vertex>& getVertices(size_t start) const { return std::vector<Vertex>(vertices.begin() + start, vertices.end());}
	//const std::vector<Vertex>& getVertices() const {return vertices;}

	void loadMesh();
	void uploadMesh();


private:

	VkPhysicalDevice PHYSICAL_DEVICE;
	VkDevice LOGICAL_DEVICE;
	VkQueue GRAPHICS_QUEUE;
	VkCommandPool COMMAND_POOL;
	VkCommandBuffer COMMAND_BUFFER;
	VkInstance INSTANCE;

	std::vector<Vertex> vertices;

	VmaAllocator ALLOCATOR;
	VmaAllocatorCreateInfo ALLOCATOR_INFO = {};

	struct AllocatedBuffer 
	{
		VkBuffer BUFFER = VK_NULL_HANDLE;
		VmaAllocation ALLOCATION = VK_NULL_HANDLE;
	};

	AllocatedBuffer VERTEX_BUFFER = {};
};


#endif // MESH_H