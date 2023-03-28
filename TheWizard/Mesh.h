#pragma once

#ifndef MESH_H
#define MESH_H

#include <Vulkan/tiny_obj_loader.h>
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <vulkan.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <string>
#include <vk_mem_alloc.h>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class Mesh
{
public:
	Mesh();
	Mesh(const char* filename);

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texcoord;
	};

	struct MeshPushConstants {
		glm::vec4 data;
		glm::mat4 render_matrix;
	}MPC;

	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> BINDINGS;
		std::vector<VkVertexInputAttributeDescription> ATTRIBUTES;
		VkPipelineVertexInputStateCreateFlags flags;
	}VID;

	VertexInputDescription description;
	MeshPushConstants pushConstants;

	void meshInit(VkPhysicalDevice PHYSICAL_DEVICE_P, VkDevice LOGICAL_DEVICE_P, VkInstance INSTACE_P, VkCommandPool COMMAND_POOL_P, VkCommandBuffer COMMAND_BUFFER_P, VkQueue GRAPHICS_QUEUE_P);

	uint32_t getVerticesSize();
	VkBuffer *getVertexBuffer();

	glm::vec4 getPushConstantsData();
	glm::mat4 getPushConstantsMatrix();
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