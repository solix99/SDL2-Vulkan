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
	Mesh(const char* filename, VkPhysicalDevice PHYSICAL_DEVICE_P, VkDevice LOGICAL_DEVICE_P, VkInstance INSTACE_P, VkQueue GRAPHICS_QUEUE_P, VmaAllocator allocator, VmaAllocatorCreateInfo ALLOCATOR_INFO);

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

	void meshInit(const char *file ,VkPhysicalDevice PHYSICAL_DEVICE_P, VkDevice LOGICAL_DEVICE_P, VkInstance INSTACE_P, VkQueue GRAPHICS_QUEUE_P, VmaAllocator ALLOCATOR, VmaAllocatorCreateInfo ALLOCATOR_INFO);

	uint32_t getVerticesSize();
	VkBuffer *getVertexBuffer();

	glm::vec4 getPushConstantsData();
	glm::mat4 getPushConstantsMatrix();
	glm::mat4 getModelMatrix();
	void loadMesh();
	void uploadMesh();
	void setMeshCoord(double x, double y, double z);

private:

	string OBJECT_NAME;

	VkPhysicalDevice PHYSICAL_DEVICE;
	VkDevice LOGICAL_DEVICE;
	VkQueue GRAPHICS_QUEUE;
	VkCommandPool COMMAND_POOL;
	VkCommandBuffer COMMAND_BUFFER;
	VkInstance INSTANCE;
	

	std::vector<Vertex> vertices;

	VmaAllocator ALLOCATOR;
	VmaAllocatorCreateInfo VmaAllocatorInfo = {};

	struct AllocatedBuffer 
	{
		VkBuffer BUFFER = VK_NULL_HANDLE;
		VmaAllocation ALLOCATION = VK_NULL_HANDLE;
	};

	AllocatedBuffer VERTEX_BUFFER = {};
};


#endif // MESH_H