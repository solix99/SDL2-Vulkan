#pragma once

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <vec3.hpp>
#include <vec2.hpp>
#include "Vulkan.h"
#include <array>
#include <glm.hpp>

class Vulkan;

class Mesh
{
public:
	Mesh(VkDevice device ,VkPhysicalDevice physical_device, VkCommandPool cmdPool, VkQueue queue);
	Mesh(Vulkan *vulkan);

	struct Vertex;

	void createVertexBuffer();
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
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkVertexInputBindingDescription bindingDescription;
	array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

	vector<Vertex> vertices;
};

#endif // MESH_H