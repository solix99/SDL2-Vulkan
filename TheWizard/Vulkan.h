#pragma once

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

using namespace std;

class Vulkan
{
public:
	Vulkan(LWindow &window);

	bool initVulkan();
	bool isDeviceSuitable(VkPhysicalDevice) const; 
	VkDevice getLogicalDevice() const;

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
	VkClearValue WINDOW_RENDER_COLOR_VK = { 1.0f, 1.0f, 1.0f, 1.0f };
	VkSubmitInfo SUBMIT_INFO_VK = {};
	VkShaderModule VERT_SHADER_MODULE = VK_NULL_HANDLE;
	VkShaderModule FRAG_SHADER_MODULE = VK_NULL_HANDLE;
	LWindow *WINDOW;

};
