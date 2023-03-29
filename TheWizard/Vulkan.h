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
#include <string>
#include <algorithm>
#include <vulkan/vulkan_core.h>

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
	~Vulkan();
	bool cleanup();
	bool initVulkan();
	bool isDeviceSuitable(VkPhysicalDevice) ; 
	VkDevice getLogicalDevice() ;
	//	create get function for COMMAND_BUFFER_VK, SWAPCHAIN_VK, SEMAPHORE_IMAGE_AVAILABLE_VK, IMAGE_INDEX_VK, SWAPCHAIN_FRAMEBUFFER_VK, SUBMIT_INFO_VK, COMMAND_BUFFER_VK
	VkCommandBuffer getCommandBuffer() ;
	VkSwapchainKHR getSwapchain() ;
	VkSemaphore getSemaphoreSignal() ;
	uint32_t *getImageIndex();
	VkFramebuffer getSwapchainFramebuffer(int i) ;
	VkSubmitInfo* getSubmitInfo();
	VkPipeline getGraphicsPipeline() ;
	VkQueue getGraphicsQueue() ;
	VkRenderPassBeginInfo *getRenderPassBeginInfo() ;
	VkCommandBufferBeginInfo *getCommandBufferBeginInfo() ;
	VkSemaphore getSemaphoreWait() const;
	VkPresentInfoKHR *getPresentInfo() ;
	VkFence *getFenceRenderingFinished();
	VkFence getFenceRenderingFinishedEx();
	VkCommandPool getCommandPool() ;
	VkPhysicalDevice getPhysicalDevice() ;
    VkInstance getInstance() ;
	void initPipeline(string name, string sShaderVertex, string sShaderFragment,Mesh & MESH);
	VkPipeline getPipeline(string name);
	VkPipeline getCurrentPipeline();
	VkPipeline switchPipeline();
	void setCurrentGraphicsPipeline(VkPipeline pipeline);
	VkPipelineLayout getPipelineLayout(Mesh & MESH);
	VkRenderPassBeginInfo getRenderPassBeginInfoEx();

	VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo imageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);

	struct AllocatedImage {
		VkImage _image;
		VmaAllocation _allocation;
	};

	VmaAllocator ALLOCATOR;
	VmaAllocatorCreateInfo ALLOCATOR_INFO = {};



	Mesh MESH;
	//Mesh MESH_MONKEY;

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

	VkBuffer VERTEX_BUFFER_VK = VK_NULL_HANDLE;
	uint32_t GRAPHICS_QUEUE_FAMILY_INDEX_VK = UINT32_MAX;
	VkCommandPool COMMAND_POOL_VK = nullptr;
	VkCommandBuffer COMMAND_BUFFER_VK = nullptr;
	VkFence FENCE_RENDERING_FINISHED_VK = nullptr;
	VkFence FENCE_IMAGE_AVAILABE_VK = nullptr;
	VkSemaphore SEMAPHORE_WAIT_VK = nullptr;
	VkSemaphore SEMAPHORE_SIGNAL_VK = nullptr;
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
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPipelineLayout meshPipelineLayout = VK_NULL_HANDLE;
	VkResult result = VK_SUCCESS;

	struct PIPELINES
	{
		size_t CURRENT = 0;
		vector <string> NAME;
		vector <VkPipeline> GRAPHICS_PIPELINES_VK;
	}PIPE;

	VkImageView _depthImageView;
	AllocatedImage _depthImage;
	VkFormat _depthFormat;
	
};

#endif