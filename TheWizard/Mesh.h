#pragma once

#include <vector>
#include <vec3.hpp>
#include <vec2.hpp>
#include "Vulkan.h"
#include <array>

class Mesh
{
public:
	Mesh(VkDevice device ,VkPhysicalDevice physical_device, VkCommandPool cmdPool, VkQueue queue);
	struct Vertex;
	vector<Vertex> vertices;
	void createVertexBuffer();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
private:

	VkDevice LOGICAL_DEVICE;
	VkPhysicalDevice PHYSICAL_DEVICE;
	VkCommandPool COMMAND_POOL;
	VkQueue QUEUE;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
};