#pragma once

#ifndef VULKAN_H
#define VULKAN_H


#include <iostream>
#include <SDL_vulkan.h>
#include <vulkan.h>
#include <GLSL.std.450.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Shader.h"
#include <SDL.h>
#include "LWindow.h"
#include <functional>
#include <deque>
#include <vk_mem_alloc.h>
#include "Mesh.h"


using namespace std;

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call the function
		}

		deletors.clear();
	}
};


class Vulkan
{
public:
	Vulkan(LWindow &window);
	Vulkan();
	bool cleanup();
	bool initVulkan();
	bool isDeviceSuitable(VkPhysicalDevice) const; 
	VkDevice getLogicalDevice() ;
	//	create get function for COMMAND_BUFFER_VK, SWAPCHAIN_VK, SEMAPHORE_IMAGE_AVAILABLE_VK, IMAGE_INDEX_VK, SWAPCHAIN_FRAMEBUFFER_VK, SUBMIT_INFO_VK, COMMAND_BUFFER_VK
	VkCommandBuffer getCommandBuffer() const;
	VkSwapchainKHR getSwapchain() const;
	VkSemaphore getSemaphoreImageAvailable() const;
	uint32_t *getImageIndex();
	VkFramebuffer getSwapchainFramebuffer(int i) const;
	VkSubmitInfo* getSubmitInfo();
	VkPipeline getGraphicsPipeline() const;
	VkQueue getGraphicsQueue() const;
	VkRenderPassBeginInfo *getRenderPassBeginInfo() ;
	VkCommandBufferBeginInfo *getCommandBufferBeginInfo() ;
	VkSemaphore getSemaphoreAvailable() const;
	VkPresentInfoKHR *getPresentInfo() ;
	VkFence *getFenceRenderingFinished();
	VkCommandPool getCommandPool() const;
	VkPhysicalDevice getPhysicalDevice() const;

	void switchPipeline();
	void setCurrentGraphicsPipeline(VkPipeline pipeline);


private:

	VkInstance INSTANCE_VK = VK_NULL_HANDLE;
	VkSurfaceKHR SURFACE_VK = VK_NULL_HANDLE;
	VkPhysicalDevice PHYSICAL_DEVICE_VK = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties DEVICE_PROPERTIES_VK = {};
	VkPhysicalDeviceFeatures DEVICE_FEATURES_VK = {};
	VkDevice LOGICAL_DEVICE_VK = VK_NULL_HANDLE;
	VkResult RESULT_VK = VK_SUCCESS;
	VkSwapchainKHR SWAPCHAIN_VK = VK_NULL_HANDLE;
	VkQueue GRAPHICS_QUEUE_VK = VK_NULL_HANDLE;
	VkQueue PRESENT_GRAPHICS_QUEUE_VK = VK_NULL_HANDLE;
	VkExtent2D SWAPCHAIN_EXTENT = {};
	VkRenderPassCreateInfo RENDER_PASS_INFO_VK = {};
	VkRenderPass RENDER_PASS_VK = VK_NULL_HANDLE;
	vector<VkFramebuffer> SWAPCHAIN_FRAMEBUFFER_VK;
	VkPipeline GRAPHICS_PIPELINE_VK = VK_NULL_HANDLE;
	VkPipeline GRAPHICS_PIPELINE_2_VK = VK_NULL_HANDLE;
	VkPipeline GRAPHICS_PIPELINE_CURRENT_VK = VK_NULL_HANDLE;
	VkBuffer VERTEX_BUFFER_VK = VK_NULL_HANDLE;
	uint32_t GRAPHICS_QUEUE_FAMILY_INDEX_VK = UINT32_MAX;
	VkCommandPool COMMAND_POOL_VK = nullptr;
	VkCommandBuffer COMMAND_BUFFER_VK = nullptr;
	VkFence FENCE_RENDERING_FINISHED_VK = nullptr;
	VkSemaphore SEMAPHORE_IMAGE_AVAILABLE_VK = nullptr;
	VkSemaphore SEMAPHORE_RENDERING_FINISHED_VK = nullptr;
	VkPresentInfoKHR PRESENT_INFO_VK = {};
	uint32_t IMAGE_INDEX_VK = 0;
	VkCommandBufferBeginInfo COMMAND_BUFFER_BEGIN_INFO_VK = {};
	VkRenderPassBeginInfo RENDER_PASS_BEGIN_INFO_VK = {};
	VkClearValue WINDOW_RENDER_COLOR_VK = { 0.0f, 0.0f, 1.0f, 1.0f };
	VkSubmitInfo SUBMIT_INFO_VK = {};
	VkShaderModule VERT_SHADER_MODULE = VK_NULL_HANDLE;
	VkShaderModule FRAG_SHADER_MODULE = VK_NULL_HANDLE;
	LWindow *WINDOW;
	DeletionQueue _mainDeletionQueue;
	VmaAllocator ALLOCATOR;
	VmaAllocatorCreateInfo ALLOCATOR_INFO = {};
	VkPipeline GRAPHICS_MESH_PIPELINE_VK;

};

#endif