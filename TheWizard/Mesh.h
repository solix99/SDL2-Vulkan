#pragma once

#ifndef MESH_H
#define MESH_H

#include <vector>
#include "Vulkan.h"
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

	//static VertexInputDescription getVertexDescription();
};

class Vulkan;


class Mesh
{
public:
	Mesh(Vulkan *vulkan);


	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	

	uint32_t getVerticesSize();
	VkBuffer *getVertexBuffer();

	const std::vector<Vertex>& getVertices(size_t start) const { return std::vector<Vertex>(vertices.begin() + start, vertices.end());}
	const std::vector<Vertex>& getVertices() const {return vertices;}

	void loadMesh();
	void uploadMesh();


private:


	Vulkan *VK;

	std::vector<Vertex> vertices;

	VmaAllocator ALLOCATOR;
	VmaAllocatorCreateInfo ALLOCATOR_INFO = {};

	struct AllocatedBuffer 
	{
		VkBuffer BUFFER;
		VmaAllocation ALLOCATION;
	};

	AllocatedBuffer VERTEX_BUFFER;
};


#endif // MESH_H