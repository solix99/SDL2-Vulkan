// TheWizard.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Hey there

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>
#include <conio.h>
#include "SDL.h"
#include <SDL_vulkan.h>
#include <SDL_mixer.h>
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Ltexture.h"
#include "Lbutton.h"
#include "LTimer.h"
#include "LWindow.h"
#include "LServer.h"
#include "LPawn.h"
#include "mysql.h"
#include "LAnim.h"
#include <SDL_thread.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <set>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <glm/glm.hpp>

WSADATA wData;
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 10000
#define MAX_PLAYER_ENTITY 4
#define MAX_PLAYER_BULLET_COUNT 28
#define SDL_GLOBAL_DELAY 10
#define FPS_LIMIT_DELAY 10
#define CLIENT_UNIQUE_ID 29

using namespace std;

#define DEFAULT_RESOLUTION_WIDTH 1280
#define DEFAULT_RESOLUTION_HEIGHT 720	

const char                      gAppName[] = "Wizard";
const char                      gEngineName[] = "SnyxEngine";
int                             gWindowWidth = DEFAULT_RESOLUTION_WIDTH;
int                             gWindowHeight = DEFAULT_RESOLUTION_HEIGHT;
VkPresentModeKHR                gPresentationMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
VkSurfaceTransformFlagBitsKHR   gTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
VkFormat                        gFormat = VK_FORMAT_B8G8R8A8_SRGB;
VkColorSpaceKHR                 gColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
VkImageUsageFlags               gImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

const std::set<std::string>& getRequestedLayerNames()
{
	static std::set<std::string> layers;
	if (layers.empty())
	{
		layers.emplace("VK_LAYER_NV_optimus");
		layers.emplace("VK_LAYER_LUNARG_standard_validation");
	}
	return layers;
}

/**
 * @return the set of required device extension names
 */
const std::set<std::string>& getRequestedDeviceExtensionNames()
{
	static std::set<std::string> layers;
	if (layers.empty())
	{
		layers.emplace(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
	return layers;
}

/**
 * @return the set of required image usage scenarios
 * that need to be supported by the surface and swap chain
 */
const std::vector<VkImageUsageFlags> getRequestedImageUsages()
{
	static std::vector<VkImageUsageFlags> usages;
	if (usages.empty())
	{
		usages.emplace_back(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	}
	return usages;
}

/**
 * Clamps value between min and max
 */
template<typename T>
T clamp(T value, T min, T max)
{
	return glm::clamp<T>(value, min, max);
}

/**
 * Callback that receives a debug message from Vulkan
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::cout << "validation layer: " << layerPrefix << ": " << msg << std::endl;
	return VK_FALSE;
}

namespace VULKAN
{
	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	/**
	 *  Sets up the vulkan messaging callback specified above
	 */
	bool setupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT& callback)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (createDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			std::cout << "unable to create debug report callback extension\n";
			return false;
		}
		return true;
	}


	/**
	 * Destroys the callback extension object
	 */
	void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
		{
			func(instance, callback, pAllocator);
		}
	}


	bool getAvailableVulkanLayers(std::vector<std::string>& outLayers)
	{
		// Figure out the amount of available layers
		// Layers are used for debugging / validation etc / profiling..
		unsigned int instance_layer_count = 0;
		VkResult res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
		if (res != VK_SUCCESS)
		{
			std::cout << "unable to query vulkan instance layer property count\n";
			return false;
		}

		std::vector<VkLayerProperties> instance_layer_names(instance_layer_count);
		res = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layer_names.data());
		if (res != VK_SUCCESS)
		{
			std::cout << "unable to retrieve vulkan instance layer names\n";
			return false;
		}

		// Display layer names and find the ones we specified above
		std::cout << "found " << instance_layer_count << " instance layers:\n";
		std::vector<const char*> valid_instance_layer_names;
		const std::set<std::string>& lookup_layers = getRequestedLayerNames();
		int count(0);
		outLayers.clear();
		for (const auto& name : instance_layer_names)
		{
			std::cout << count << ": " << name.layerName << ": " << name.description << "\n";
			auto it = lookup_layers.find(std::string(name.layerName));
			if (it != lookup_layers.end())
				outLayers.emplace_back(name.layerName);
			count++;
		}

		// Print the ones we're enabling
		std::cout << "\n";
		for (const auto& layer : outLayers)
			std::cout << "applying layer: " << layer.c_str() << "\n";
		return true;
	}


	bool getAvailableVulkanExtensions(SDL_Window* window, std::vector<std::string>& outExtensions)
	{
		// Figure out the amount of extensions vulkan needs to interface with the os windowing system
		// This is necessary because vulkan is a platform agnostic API and needs to know how to interface with the windowing system
		unsigned int ext_count = 0;
		if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr))
		{
			std::cout << "Unable to query the number of Vulkan instance extensions\n";
			return false;
		}

		// Use the amount of extensions queried before to retrieve the names of the extensions
		std::vector<const char*> ext_names(ext_count);
		if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names.data()))
		{
			std::cout << "Unable to query the number of Vulkan instance extension names\n";
			return false;
		}

		// Display names
		std::cout << "found " << ext_count << " Vulkan instance extensions:\n";
		for (unsigned int i = 0; i < ext_count; i++)
		{
			std::cout << i << ": " << ext_names[i] << "\n";
			outExtensions.emplace_back(ext_names[i]);
		}

		// Add debug display extension, we need this to relay debug messages
		outExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		std::cout << "\n";
		return true;
	}


	/**
	 * Creates a vulkan instance using all the available instance extensions and layers
	 * @return if the instance was created successfully
	 */
	bool createVulkanInstance(const std::vector<std::string>& layerNames, const std::vector<std::string>& extensionNames, VkInstance& outInstance)
	{
		// Copy layers
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Copy extensions
		std::vector<const char*> ext_names;
		for (const auto& ext : extensionNames)
			ext_names.emplace_back(ext.c_str());

		// Get the suppoerted vulkan instance version
		unsigned int api_version;
		vkEnumerateInstanceVersion(&api_version);

		// initialize the VkApplicationInfo structure
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = gAppName;
		app_info.applicationVersion = 1;
		app_info.pEngineName = gEngineName;
		app_info.engineVersion = 1;
		app_info.apiVersion = VK_API_VERSION_1_0;

		// initialize the VkInstanceCreateInfo structure
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledExtensionCount = static_cast<uint32_t>(ext_names.size());
		inst_info.ppEnabledExtensionNames = ext_names.data();
		inst_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		inst_info.ppEnabledLayerNames = layer_names.data();

		// Create vulkan runtime instance
		std::cout << "initializing Vulkan instance\n\n";
		VkResult res = vkCreateInstance(&inst_info, NULL, &outInstance);
		switch (res)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			std::cout << "unable to create vulkan instance, cannot find a compatible Vulkan ICD\n";
			return false;
		default:
			std::cout << "unable to create Vulkan instance: unknown error\n";
			return false;
		}
		return true;
	}


	/**
	 * Allows the user to select a GPU (physical device)
	 * @return if query, selection and assignment was successful
	 * @param outDevice the selected physical device (gpu)
	 * @param outQueueFamilyIndex queue command family that can handle graphics commands
	 */
	bool selectGPU(VkInstance instance, VkPhysicalDevice& outDevice, unsigned int& outQueueFamilyIndex)
	{
		// Get number of available physical devices, needs to be at least 1
		unsigned int physical_device_count(0);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		if (physical_device_count == 0)
		{
			std::cout << "No physical devices found\n";
			return false;
		}

		// Now get the devices
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Show device information
		std::cout << "found " << physical_device_count << " GPU(s):\n";
		int count(0);
		std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_devices.size());
		for (auto& physical_device : physical_devices)
		{
			vkGetPhysicalDeviceProperties(physical_device, &(physical_device_properties[count]));
			std::cout << count << ": " << physical_device_properties[count].deviceName << "\n";
			count++;
		}

		// Select one if more than 1 is available
		unsigned int selection_id = 0;
		if (physical_device_count > 1)
		{
			while (true)
			{
				std::cout << "select device: ";
				std::cin >> selection_id;
				if (selection_id >= physical_device_count || selection_id < 0)
				{
					std::cout << "invalid selection, expected a value between 0 and " << physical_device_count - 1 << "\n";
					continue;
				}
				break;
			}
		}
		std::cout << "selected: " << physical_device_properties[selection_id].deviceName << "\n";
		VkPhysicalDevice selected_device = physical_devices[selection_id];

		// Find the number queues this device supports, we want to make sure that we have a queue that supports graphics commands
		unsigned int family_queue_count(0);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, nullptr);
		if (family_queue_count == 0)
		{
			std::cout << "device has no family of queues associated with it\n";
			return false;
		}

		// Extract the properties of all the queue families
		std::vector<VkQueueFamilyProperties> queue_properties(family_queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, queue_properties.data());

		// Make sure the family of commands contains an option to issue graphical commands.
		unsigned int queue_node_index = -1;
		for (unsigned int i = 0; i < family_queue_count; i++)
		{
			if (queue_properties[i].queueCount > 0 && queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queue_node_index = i;
				break;
			}
		}

		if (queue_node_index < 0)
		{
			std::cout << "Unable to find a queue command family that accepts graphics commands\n";
			return false;
		}

		// Set the output variables
		outDevice = selected_device;
		outQueueFamilyIndex = queue_node_index;
		return true;
	}


	/**
	 *  Creates a logical device
	 */
	bool createLogicalDevice(VkPhysicalDevice& physicalDevice,
		unsigned int queueFamilyIndex,
		const std::vector<std::string>& layerNames,
		VkDevice& outDevice)
	{
		// Copy layer names
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());


		// Get the number of available extensions for our graphics card
		uint32_t device_property_count(0);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, NULL) != VK_SUCCESS)
		{
			std::cout << "Unable to acquire device extension property count\n";
			return false;
		}
		std::cout << "\nfound " << device_property_count << " device extensions\n";

		// Acquire their actual names
		std::vector<VkExtensionProperties> device_properties(device_property_count);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, device_properties.data()) != VK_SUCCESS)
		{
			std::cout << "Unable to acquire device extension property names\n";
			return false;
		}

		// Match names against requested extension
		std::vector<const char*> device_property_names;
		const std::set<std::string>& required_extension_names = getRequestedDeviceExtensionNames();
		int count = 0;
		for (const auto& ext_property : device_properties)
		{
			std::cout << count << ": " << ext_property.extensionName << "\n";
			auto it = required_extension_names.find(std::string(ext_property.extensionName));
			if (it != required_extension_names.end())
			{
				device_property_names.emplace_back(ext_property.extensionName);
			}
			count++;
		}

		// Warn if not all required extensions were found
		if (required_extension_names.size() != device_property_names.size())
		{
			std::cout << "not all required device extensions are supported!\n";
			return false;
		}

		std::cout << "\n";
		for (const auto& name : device_property_names)
			std::cout << "applying device extension: " << name << "\n";

		// Create queue information structure used by device based on the previously fetched queue information from the physical device
		// We create one command processing queue for graphics
		VkDeviceQueueCreateInfo queue_create_info;
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queueFamilyIndex;
		queue_create_info.queueCount = 1;
		std::vector<float> queue_prio = { 1.0f };
		queue_create_info.pQueuePriorities = queue_prio.data();
		queue_create_info.pNext = NULL;
		queue_create_info.flags = NULL;

		// Device creation information
		VkDeviceCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = 1;
		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.ppEnabledLayerNames = layer_names.data();
		create_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		create_info.ppEnabledExtensionNames = device_property_names.data();
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_property_names.size());
		create_info.pNext = NULL;
		create_info.pEnabledFeatures = NULL;
		create_info.flags = NULL;

		// Finally we're ready to create a new device
		VkResult res = vkCreateDevice(physicalDevice, &create_info, nullptr, &outDevice);
		if (res != VK_SUCCESS)
		{
			std::cout << "failed to create logical device!\n";
			return false;
		}
		return true;
	}


	/**
	 *  Returns the vulkan device queue associtated with the previously created device
	 */
	void getDeviceQueue(VkDevice device, int familyQueueIndex, VkQueue& outGraphicsQueue)
	{
		vkGetDeviceQueue(device, familyQueueIndex, 0, &outGraphicsQueue);
	}


	/**
	 *  Creates the vulkan surface that is rendered to by the device using SDL
	 */
	bool createSurface(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, uint32_t graphicsFamilyQueueIndex, VkSurfaceKHR& outSurface)
	{
		if (!SDL_Vulkan_CreateSurface(window, instance, &outSurface))
		{
			std::cout << "Unable to create Vulkan compatible surface using SDL\n";
			return false;
		}

		// Make sure the surface is compatible with the queue family and gpu
		VkBool32 supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, graphicsFamilyQueueIndex, outSurface, &supported);
		if (!supported)
		{
			std::cout << "Surface is not supported by physical device!\n";
			return false;
		}

		return true;
	}


	/**
	 * @return if the present modes could be queried and ioMode is set
	 * @param outMode the mode that is requested, will contain FIFO when requested mode is not available
	 */
	bool getPresentationMode(VkSurfaceKHR surface, VkPhysicalDevice device, VkPresentModeKHR& ioMode)
	{
		uint32_t mode_count(0);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, NULL) != VK_SUCCESS)
		{
			std::cout << "unable to query present mode count for physical device\n";
			return false;
		}

		std::vector<VkPresentModeKHR> available_modes(mode_count);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, available_modes.data()) != VK_SUCCESS)
		{
			std::cout << "unable to query the various present modes for physical device\n";
			return false;
		}

		for (auto& mode : available_modes)
		{
			if (mode == ioMode)
				return true;
		}
		std::cout << "unable to obtain preferred display mode, fallback to FIFO\n";
		ioMode = VK_PRESENT_MODE_FIFO_KHR;
		return true;
	}


	/**
	 * Obtain the surface properties that are required for the creation of the swap chain
	 */
	bool getSurfaceProperties(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities) != VK_SUCCESS)
		{
			std::cout << "unable to acquire surface capabilities\n";
			return false;
		}
		return true;
	}


	/**
	 * Figure out the number of images that are used by the swapchain and
	 * available to us in the application, based on the minimum amount of necessary images
	 * provided by the capabilities struct.
	 */
	unsigned int getNumberOfSwapImages(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		unsigned int number = capabilities.minImageCount + 1;
		return number > capabilities.maxImageCount ? capabilities.minImageCount : number;
	}


	/**
	 *  Returns the size of a swapchain image based on the current surface
	 */
	VkExtent2D getSwapImageSize(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		// Default size = window size
		VkExtent2D size = { (unsigned int)gWindowWidth, (unsigned int)gWindowHeight };

		// This happens when the window scales based on the size of an image
		if (capabilities.currentExtent.width == 0xFFFFFFF)
		{
			size.width = glm::clamp<unsigned int>(size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			size.height = glm::clamp<unsigned int>(size.height, capabilities.maxImageExtent.height, capabilities.maxImageExtent.height);
		}
		else
		{
			size = capabilities.currentExtent;
		}
		return size;
	}


	/**
	 * Checks if the surface supports color and other required surface bits
	 * If so constructs a ImageUsageFlags bitmask that is returned in outUsage
	 * @return if the surface supports all the previously defined bits
	 */
	bool getImageUsage(const VkSurfaceCapabilitiesKHR& capabilities, VkImageUsageFlags& outUsage)
	{
		const std::vector<VkImageUsageFlags>& desir_usages = getRequestedImageUsages();
		assert(desir_usages.size() > 0);

		// Needs to be always present
		outUsage = desir_usages[0];

		for (const auto& desired_usage : desir_usages)
		{
			VkImageUsageFlags image_usage = desired_usage & capabilities.supportedUsageFlags;
			if (image_usage != desired_usage)
			{
				std::cout << "unsupported image usage flag: " << desired_usage << "\n";
				return false;
			}

			// Add bit if found as supported color
			outUsage = (outUsage | desired_usage);
		}

		return true;
	}


	/**
	 * @return transform based on global declared above, current transform if that transform isn't available
	 */
	VkSurfaceTransformFlagBitsKHR getTransform(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.supportedTransforms & gTransform)
			return gTransform;
		std::cout << "unsupported surface transform: " << gTransform;
		return capabilities.currentTransform;
	}


	/**
	 * @return the most appropriate color space based on the globals provided above
	 */
	bool getFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR& outFormat)
	{
		unsigned int count(0);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr) != VK_SUCCESS)
		{
			std::cout << "unable to query number of supported surface formats";
			return false;
		}

		std::vector<VkSurfaceFormatKHR> found_formats(count);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, found_formats.data()) != VK_SUCCESS)
		{
			std::cout << "unable to query all supported surface formats\n";
			return false;
		}

		// This means there are no restrictions on the supported format.
		// Preference would work
		if (found_formats.size() == 1 && found_formats[0].format == VK_FORMAT_UNDEFINED)
		{
			outFormat.format = gFormat;
			outFormat.colorSpace = gColorSpace;
			return true;
		}

		// Otherwise check if both are supported
		for (const auto& found_format_outer : found_formats)
		{
			// Format found
			if (found_format_outer.format == gFormat)
			{
				outFormat.format = found_format_outer.format;
				for (const auto& found_format_inner : found_formats)
				{
					// Color space found
					if (found_format_inner.colorSpace == gColorSpace)
					{
						outFormat.colorSpace = found_format_inner.colorSpace;
						return true;
					}
				}

				// No matching color space, pick first one
				std::cout << "warning: no matching color space found, picking first available one\n!";
				outFormat.colorSpace = found_formats[0].colorSpace;
				return true;
			}
		}

		// No matching formats found
		std::cout << "warning: no matching color format found, picking first available one\n";
		outFormat = found_formats[0];
		return true;
	}


	/**
	 * creates the swap chain using utility functions above to retrieve swap chain properties
	 * Swap chain is associated with a single window (surface) and allows us to display images to screen
	 */
	bool createSwapChain(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, VkSwapchainKHR& outSwapChain)
	{
		// Get properties of surface, necessary for creation of swap-chain
		VkSurfaceCapabilitiesKHR surface_properties;
		if (!getSurfaceProperties(physicalDevice, surface, surface_properties))
			return false;

		// Get the image presentation mode (synced, immediate etc.)
		VkPresentModeKHR presentation_mode = gPresentationMode;
		if (!getPresentationMode(surface, physicalDevice, presentation_mode))
			return false;

		// Get other swap chain related features
		unsigned int swap_image_count = getNumberOfSwapImages(surface_properties);

		// Size of the images
		VkExtent2D swap_image_extent = getSwapImageSize(surface_properties);

		// Get image usage (color etc.)
		VkImageUsageFlags usage_flags;
		if (!getImageUsage(surface_properties, usage_flags))
			return false;

		// Get the transform, falls back on current transform when transform is not supported
		VkSurfaceTransformFlagBitsKHR transform = getTransform(surface_properties);

		// Get swapchain image format
		VkSurfaceFormatKHR image_format;
		if (!getFormat(physicalDevice, surface, image_format))
			return false;

		// Old swap chain
		VkSwapchainKHR old_swap_chain = outSwapChain;

		// Populate swapchain creation info
		VkSwapchainCreateInfoKHR swap_info;
		swap_info.pNext = nullptr;
		swap_info.flags = 0;
		swap_info.surface = surface;
		swap_info.minImageCount = swap_image_count;
		swap_info.imageFormat = image_format.format;
		swap_info.imageColorSpace = image_format.colorSpace;
		swap_info.imageExtent = swap_image_extent;
		swap_info.imageArrayLayers = 1;
		swap_info.imageUsage = usage_flags;
		swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_info.queueFamilyIndexCount = 0;
		swap_info.pQueueFamilyIndices = nullptr;
		swap_info.preTransform = transform;
		swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swap_info.presentMode = presentation_mode;
		swap_info.clipped = true;
		swap_info.oldSwapchain = NULL;
		swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		// Destroy old swap chain
		if (old_swap_chain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, old_swap_chain, nullptr);
			old_swap_chain = VK_NULL_HANDLE;
		}

		// Create new one
		if (vkCreateSwapchainKHR(device, &swap_info, nullptr, &old_swap_chain) != VK_SUCCESS)
		{
			std::cout << "unable to create swap chain\n";
			return false;
		}

		// Store handle
		outSwapChain = old_swap_chain;
		return true;
	}


	/**
	 *  Returns the handles of all the images in a swap chain, result is stored in outImageHandles
	 */
	bool getSwapChainImageHandles(VkDevice device, VkSwapchainKHR chain, std::vector<VkImage>& outImageHandles)
	{
		unsigned int image_count(0);
		VkResult res = vkGetSwapchainImagesKHR(device, chain, &image_count, nullptr);
		if (res != VK_SUCCESS)
		{
			std::cout << "unable to get number of images in swap chain\n";
			return false;
		}

		outImageHandles.clear();
		outImageHandles.resize(image_count);
		if (vkGetSwapchainImagesKHR(device, chain, &image_count, outImageHandles.data()) != VK_SUCCESS)
		{
			std::cout << "unable to get image handles from swap chain\n";
			return false;
		}
		return true;
	}

	/**
	 *  Destroys the vulkan instance
	 */
	void vulkanQuit(VkInstance instance, VkDevice device, VkDebugReportCallbackEXT callback, VkSwapchainKHR chain)
	{
		vkDestroySwapchainKHR(device, chain, nullptr);
		vkDestroyDevice(device, nullptr);
		destroyDebugReportCallbackEXT(instance, callback, nullptr);
		vkDestroyInstance(instance, nullptr);
		SDL_Quit();
	}

}

struct engineThreads
{
	SDL_Thread* recvThread = NULL;
	SDL_Thread* SEND_DATA = NULL;
	SDL_Thread* PHYSICS = NULL;

}THREAD;


struct engineParameters
{
	struct FSTREAM
	{
		fstream gameLog;
	}FSTR;

	struct ANIMATION
	{
		int FIREBALL_RENDERSPEED = 500;

	}ANIM;

	struct GAMESYSTEM
	{
		LTimer physicsTimer;
		LTimer physicsTimerMovement;
		LTimer fpsTimer;
		LTimer exitLoopTimer;
		int physicsRate = 20;
	}GSYS;

	struct BOOLEAN
	{
		bool removeClient = false;
		bool addClient = false;
		bool playerDamage = false;
		bool injectProjectile = false;
		bool renderCollisionBox = false;
		bool isMatching = false;
		bool inMatchingScreen = false;
		bool exitCurrentLoop = false;
		bool isSendThreadActive = false;
		bool isPhysicsThreadActive = false;
		bool isReciveThreadActive = false;

	}EXECUTE;
	struct TEMPORAL
	{
		int projectileX;
		int projectileY;
		int	projectileDX;
		int	projectileDY;
		int gameFps;
		int damageAmount;
		int projIdentifier;
		int matchingType;

		stringstream miscSS;
		stringstream DATAPACKET;
		stringstream DATAPACKET_DEFAULT;
	}TEMP;

}EP;

struct MEMEORY
{
	struct TEXTURES
	{
		LTexture miscText;
		LTexture fpsText;
		LTexture PAWN_COLLISION_REFERENCE;
		LTexture MATCHING_BUTTON_CHOICE;
		LTexture MATCHING_IN_PROGRESS;
	}TEXTR;
	struct FONT
	{
		TTF_Font* gNorthFontLarge = NULL;
	}FNT;
	struct BUTTON
	{
		LButton TWO_BUTTON;
		LButton FOUR_BUTTON;
	}BTT;

}MEM;

				
enum PhysicsType
{
	PHYSICS_TYPE_PROJECTILES,
	PHYSICS_TYPE_PLAYER_MOVEMENT
};

enum INDENTIFIER_TYPE
{
	GET_DATA_ABOUT_PLAYER,
	DELETE_PLAYER,
	NEW_PLAYER,
	DAMAGE_PLAYER,
	KILL_PLAYER,
	UPDATE_BULLET,
	START_MATCHMAKING,
	MATCHING_COMPLETE,
	MATCH_RESULT,
	END_OF_PACKET,
	SET_POSITION
};


enum PROJECTILE_IDENTIFIER
{
	PROJ_KILLSHOT,
	PROJ_NORMALSHOT
};

enum MATCHING_TYPE
{
	TWO_PLAYER,
	FOUR_PLAYER
};


int iResult;
struct addrinfo* result = NULL, * ptr = NULL, hints;
SOCKET ConnectSocket;
bool kickPlayerDuplicate = false;

SDL_RendererFlip flipType = SDL_FLIP_HORIZONTAL;
int mouseX = 0, mouseY = 0;

bool bClientProjContact = false;
int enemyContactIdentifier;
int enemyContactProjIdentifier;
int sendPacket(void* ptr);
int recvbuflen = DEFAULT_BUFLEN;

char sendbuf[DEFAULT_BUFLEN];
char recvbuf[DEFAULT_BUFLEN];

int sResult;
stringstream fpsSS;
int frame = 0;

string input, sFlipType, sAnimType;
stringstream tempss;
stringstream connectInfo;
stringstream duplicateInfo;

SDL_Renderer* gRenderer = NULL;
Mix_Music* gMusic = NULL;
Mix_Chunk* bluebullet_sound = NULL;
TTF_Font* gFont = NULL;
TTF_Font* gNorthFont = NULL;
SDL_Event e;

LServer gServer;
LWindow gWindow(DEFAULT_RESOLUTION_WIDTH, DEFAULT_RESOLUTION_HEIGHT);

LPawn CLIENT;
LPawn Player[MAX_PLAYER_ENTITY];

SDL_Color gColor = { 0,0,0 };

LTexture loginPage_texture;
LTexture user_text_texture;
LTexture pass_text_texture;
LTexture green_textbox_texture;
LTexture red_textbox_texture;
LTexture info_text_texture;
LTexture background_texture;
LTexture crosshair_texture;
LTexture nickname_text_texture;
LTexture texture_brickFloor;

LAnim ANIM_RUNNING;
LAnim ANIM_WALKING;
LAnim ANIM_IDLE;
LAnim ANIM_RUNNING_ATTACK;
LAnim ANIM_FIREBALL;
LAnim ANIM_CONTACT_REDEXPLOSION;

LButton user_textbox_button;
LButton pass_textbox_button;
LButton login_button;
LButton register_button;

LTimer typeLine_timer;
LTimer popup_timer;
LTimer fireball_attack_timer;

float scaleX = 0.5625f;
float scaleY = 1.7777f;
int xLast = 0, yLast = 0;
int projectileX, projectileY, projectileDX, projectileDY, velX = 0, velY = 0, animCollisionX, animCollisionY;
bool collisionFound = false, addNewCollisionAnim = false;

bool init();
bool loadMedia();
void close();
bool checkCollision(SDL_Rect a, SDL_Rect b);
bool injectProjectile = false;
bool quit = false;
bool bServerThread = true;
bool connectToGameServer();
void handleCollision();
void renderTextures();
bool getPhysicsReady(int type);
int processPhysics(void* ptr);
int f_processMisc(void* ptr);


static int processPhysics(void* ptr)
{
	while (true)
	{
		if (EP.EXECUTE.isPhysicsThreadActive)
		{
			if (getPhysicsReady(PHYSICS_TYPE_PLAYER_MOVEMENT))
			{
				CLIENT.handleEvent(e);

				if (ANIM_RUNNING_ATTACK.getInUse())
				{
					if (ANIM_RUNNING_ATTACK.getCurrentTickClient(0, CLIENT_UNIQUE_ID) > ANIM_RUNNING_ATTACK.getTickCount() - 3)
					{
						ANIM_RUNNING_ATTACK.setCurrentTickClient(0, CLIENT_UNIQUE_ID, 0);
						ANIM_RUNNING_ATTACK.setInUse(false);
					}
					CLIENT.setAnimType("runAttack");
					ANIM_RUNNING_ATTACK.updateAnim(0, CLIENT_UNIQUE_ID);
				}
				else if (CLIENT.getIfMoving())
				{
					CLIENT.setAnimType("walking");
					ANIM_WALKING.updateAnim(0, CLIENT_UNIQUE_ID);
				}
				else
				{
					CLIENT.setAnimType("idle");
					ANIM_IDLE.updateAnim(0, CLIENT_UNIQUE_ID);
				}

				//Handle collision

				handleCollision();
			}
			if (getPhysicsReady(PHYSICS_TYPE_PROJECTILES))
			{
				if (CLIENT.getProjectileActive())
				{
					for (unsigned int i = 0; i < MAX_PLAYER_BULLET_COUNT; i++)
					{
						if (!CLIENT.gProjectile[i].getSlotFree())
						{
							CLIENT.gProjectile[i].checkStatus();
							ANIM_FIREBALL.updateAnim(i, CLIENT_UNIQUE_ID);
							CLIENT.gProjectile[i].setCollisionOffset(CLIENT.gProjectile[i].getXCollisionOffset() + 1.0f, CLIENT.gProjectile[i].getYCollisionOffset() + 1);

							if (!ANIM_FIREBALL.getSeqInUse(i, CLIENT_UNIQUE_ID))
							{
								CLIENT.gProjectile[i].setSlotFree(true);
							}
						}
					}
				}
				for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
				{
					if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
					{
						for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
						{
							if (!Player[i].gProjectile[j].getSlotFree())
							{
								ANIM_FIREBALL.updateAnim(i, j);
								Player[i].gProjectile[j].checkStatus();
								Player[i].gProjectile[j].setCollisionOffset(Player[i].gProjectile[j].getXCollisionOffset() + 1.0f, Player[i].gProjectile[j].getYCollisionOffset() + 1);

								if (!ANIM_FIREBALL.getSeqInUse(i, j))
								{
									Player[i].gProjectile[j].setSlotFree(true);
								}
							}
						}
					}
				}
			}

			//Handle collision

			handleCollision();
		}
		SDL_Delay(SDL_GLOBAL_DELAY);
	}
}

bool initVulkan()
{
	std::vector<std::string> found_extensions;
	if (!VULKAN::getAvailableVulkanExtensions(gWindow.getWindow(), found_extensions))
		return -1;

	// Get available vulkan layer extensions, notify when not all could be found
	std::vector<std::string> found_layers;
	if (!VULKAN::getAvailableVulkanLayers(found_layers))
		return -1;

	// Warn when not all requested layers could be found
	if (found_layers.size() != getRequestedLayerNames().size())
		std::cout << "warning! not all requested layers could be found!\n";

	// Create Vulkan Instance
	if (!VULKAN::createVulkanInstance(found_layers, found_extensions, gWindow.getVulkanInstance()))
		return -1;

	// Vulkan messaging callback
	VkDebugReportCallbackEXT callback;
	VULKAN::setupDebugCallback(gWindow.getVulkanInstance(), callback);

	// Select GPU after succsessful creation of a vulkan instance (jeeeej no global states anymore)
	VkPhysicalDevice gpu;
	unsigned int graphics_queue_index(-1);
	if (!VULKAN::selectGPU(gWindow.getVulkanInstance(), gpu, graphics_queue_index))
		return -1;

	// Create a logical device that interfaces with the physical device
	VkDevice device;
	if (!VULKAN::createLogicalDevice(gpu, graphics_queue_index, found_layers, device))
		return -1;

	// Create the surface we want to render to, associated with the window we created before
	// This call also checks if the created surface is compatible with the previously selected physical device and associated render queue
	VkSurfaceKHR presentation_surface;
	if (!VULKAN::createSurface(gWindow.getWindow(), gWindow.getVulkanInstance(), gpu, graphics_queue_index, presentation_surface))
		return -1;

	// Create swap chain
	VkSwapchainKHR swap_chain = NULL;
	if (!VULKAN::createSwapChain(presentation_surface, gpu, device, swap_chain))
		return -1;

	// Get image handles from swap chain
	std::vector<VkImage> chain_images;
	if (!VULKAN::getSwapChainImageHandles(device, swap_chain, chain_images))
		return -1;

	// Fetch the queue we want to submit the actual commands to
	VkQueue graphics_queue;
	VULKAN::getDeviceQueue(device, graphics_queue_index, graphics_queue);


	return true;
}

bool getPhysicsReady(int type)
{
	if (PHYSICS_TYPE_PROJECTILES == type)
	{
		if (EP.GSYS.physicsTimer.getTicks() > EP.GSYS.physicsRate)
		{
			EP.GSYS.physicsTimer.reset();
			return true;
		}
	}
	else if (PHYSICS_TYPE_PLAYER_MOVEMENT == type)
	{
		if (EP.GSYS.physicsTimerMovement.getTicks() > EP.GSYS.physicsRate)
		{
			EP.GSYS.physicsTimerMovement.reset();
			return true;
		}
	}

	return false;
}

void tryLoopExit()
{
	if (EP.GSYS.exitLoopTimer.getTicks() > 1000)
	{
		EP.EXECUTE.exitCurrentLoop = true;
		EP.GSYS.exitLoopTimer.reset();
	}
}
void resetPlayerData()
{
	for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed())
		{
			Player[i].resetData();
		}
	}
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}

void handleCollision()
{
	//CHEKING FOR PLAYER COLLISION
	
	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
		{
			if (checkCollision(Player[i].getCollisionRect(), CLIENT.getCollisionRect()))
			{
				CLIENT.setPosX(xLast);
				CLIENT.setPosY(yLast);
				collisionFound = true;


				break;
			}
		}
	}

	if (!collisionFound)
	{
		xLast = CLIENT.getPosX();
		yLast = CLIENT.getPosY();
	}
	
	collisionFound = false;

	//CHECKING FOR BULLLET COLLSISION OF OTHER PLAYERS

	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
		{
			for (unsigned int k = 0; k < MAX_PLAYER_ENTITY; k++)
			{
				if (Player[k].getIfSlotUsed() && Player[k].getID() != Player[i].getID())
				{
					for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
					{
						if (!Player[i].gProjectile[j].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(i,j))
						{
							for (unsigned int l = 0; l < MAX_PLAYER_BULLET_COUNT; l++)
							{
								if (!Player[k].gProjectile[l].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(k, l))
								{
									if (checkCollision(Player[i].gProjectile[j].getCollisionRect(), Player[k].gProjectile[l].getCollisionRect()))
									{
										Player[i].gProjectile[j].setSlotFree(true);
										Player[k].gProjectile[l].setSlotFree(true);

										ANIM_FIREBALL.setCurrentTickClient(i, j, 0);
										ANIM_FIREBALL.setCurrentTickClient(k, l, 0);

										ANIM_FIREBALL.setIsInverseSeq(i, j, false);
										ANIM_FIREBALL.setIsInverseSeq(k, l, false);

										ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim((Player[i].gProjectile[j].getPosX() + Player[k].gProjectile[l].getPosX()) / 2, (Player[i].gProjectile[j].getPosY() + Player[k].gProjectile[l].getPosY()) / 2, true, false);
									}
								}
							}
						}
					}
				}
			}
			//COLLISION BETWEEN CLIENT AND PLAYERS

			for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
			{
				if (!CLIENT.gProjectile[j].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(j, CLIENT_UNIQUE_ID))
				{
					for (unsigned int k = 0; k < MAX_PLAYER_BULLET_COUNT; k++)
					{
						if (!Player[i].gProjectile[k].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(i, k))
						{
							if (checkCollision(Player[i].gProjectile[k].getCollisionRect(), CLIENT.gProjectile[j].getCollisionRect()))
							{

								Player[i].gProjectile[k].setSlotFree(true);
								CLIENT.gProjectile[j].setSlotFree(true);

								ANIM_FIREBALL.setIsInverseSeq(i, k, false);
								ANIM_FIREBALL.setIsInverseSeq(i, CLIENT_UNIQUE_ID, false);

								ANIM_FIREBALL.setCurrentTickClient(i, k, 0);
								ANIM_FIREBALL.setCurrentTickClient(i, CLIENT_UNIQUE_ID, 0);

								ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim((Player[i].gProjectile[k].getPosX() + CLIENT.gProjectile[j].getPosX()) / 2, (Player[i].gProjectile[k].getPosY() + CLIENT.gProjectile[j].getPosY()) / 2, true, false);
							}
						}
					}
				}
			}
			//CHECK FOR COLISIONS BETWEEN CLIENT AND PLAYER BULLETS

			for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
			{
				if (!Player[i].gProjectile[j].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(i, j))
				{
					if (checkCollision(Player[i].gProjectile[j].getCollisionRect(), CLIENT.getCollisionRect()))
					{
						 CLIENT.damageTarget(Player[i].gProjectile[j].getDMG());
						
						 Player[i].gProjectile[j].setSlotFree(true);
						 ANIM_FIREBALL.setCurrentTickClient(i,j,0);
						 ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim((Player[i].gProjectile[j].getPosX() + CLIENT.getCollisionRect().x) / 2, (Player[i].gProjectile[j].getPosY() + CLIENT.getCollisionRect().y) / 2, true, false);
 					}
				}
			}

			//COLLISION BETWEEN CLEINT PROJ AND PLAYERS

			for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
			{
				if (!CLIENT.gProjectile[j].getSlotFree())
				{
					if (checkCollision(CLIENT.gProjectile[j].getCollisionRect(), Player[i].getCollisionRect()))
					{
						Player[i].damageTarget(CLIENT.gProjectile[j].getDMG());

						CLIENT.gProjectile[j].setSlotFree(true);
						ANIM_FIREBALL.setCurrentTickClient(j, CLIENT_UNIQUE_ID, 0);
						ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim((CLIENT.gProjectile[j].getCollisionRect().x + Player[i].getCollisionRect().x) / 2, (CLIENT.gProjectile[j].getCollisionRect().y + Player[i].getCollisionRect().y) / 2, true, false);
						
					}
				}
			} 

			//CHECK FOR COLLISION BETWEEN PLAYERS MODEL AND PLAYERS BULLETS

			for (unsigned int j = 0; j < MAX_PLAYER_ENTITY; j++)
			{
				if (Player[j].getIfSlotUsed() && i != j)
				{
					for (unsigned int k = 0; k < MAX_PLAYER_BULLET_COUNT; k++)
					{
						if (!Player[j].gProjectile[k].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(j, k))
						{
							if (checkCollision(Player[i].getCollisionRect(), Player[j].gProjectile[k].getCollisionRect()))
							{
								Player[i].damageTarget(Player[j].gProjectile[k].getDMG());

								Player[j].gProjectile[k].setSlotFree(true);
								ANIM_FIREBALL.setCurrentTickClient(j, k, 0);
								ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim((Player[i].getCollisionRect().x + Player[j].gProjectile[k].getCollisionRect().x) / 2, (Player[i].getCollisionRect().y + Player[j].gProjectile[k].getCollisionRect().y) / 2, true, false);
							} 
						}
					}
				}
			}
		}
	}
}

int clientSendData(const string &input)
{
	if (strlen(input.c_str()) < DEFAULT_BUFLEN)
	{
		iResult = send(ConnectSocket, input.c_str(), (int)strlen(input.c_str()), 0);
		if (iResult == SOCKET_ERROR) {
			cout << endl << "send failed:" << WSAGetLastError();
		}
		return iResult;
	}
}

void renderTextures()
{
	flipType = CLIENT.getCharDir() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	CLIENT.setFlipTypeString(sFlipType = flipType == SDL_FLIP_HORIZONTAL ? "horizontal" : "none");

	SDL_RenderClear(gWindow.getRenderer());
	SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

	//RENDER GROUND

	texture_brickFloor.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth(),0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
	texture_brickFloor.render(gWindow.getRenderer(), 0 ,texture_brickFloor.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth(), texture_brickFloor.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth()*2, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth()*2, texture_brickFloor.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

	//RENDER CLIENT PLAYER

	if (ANIM_RUNNING_ATTACK.getInUse())
	{
		ANIM_RUNNING_ATTACK.renderTexture(gWindow.getRenderer(), CLIENT.getPosX(), CLIENT.getPosY(), 0, 0, CLIENT_UNIQUE_ID,false, flipType, 500,50,EP.EXECUTE.renderCollisionBox,CLIENT.getCollisionRect().w,CLIENT.getCollisionRect().h, CLIENT.getCollisionRect().x, CLIENT.getCollisionRect().y);
		CLIENT.setAnimType("runAttack");
	}
	else if (CLIENT.getIfMoving())
	{
		ANIM_WALKING.renderTexture(gWindow.getRenderer(), CLIENT.getPosX(), CLIENT.getPosY(), 0, 0, CLIENT_UNIQUE_ID,false, flipType, 500,500,EP.EXECUTE.renderCollisionBox,CLIENT.getCollisionRect().w,CLIENT.getCollisionRect().h, CLIENT.getCollisionRect().x, CLIENT.getCollisionRect().y);
		CLIENT.setAnimType("walking");
	}
	else
	{
		ANIM_IDLE.renderTexture(gWindow.getRenderer(), CLIENT.getPosX(), CLIENT.getPosY(), 0, 0 , CLIENT_UNIQUE_ID,false, flipType, 500,500,EP.EXECUTE.renderCollisionBox,CLIENT.getCollisionRect().w,CLIENT.getCollisionRect().h, CLIENT.getCollisionRect().x, CLIENT.getCollisionRect().y);
		CLIENT.setAnimType("idle");
	}

	//RENDER PLAYER PROJECTILES

	if (CLIENT.getProjectileActive())
	{
		for (unsigned int i = 0; i < MAX_PLAYER_BULLET_COUNT; i++)
		{
			if (!CLIENT.gProjectile[i].getSlotFree())
			{
				ANIM_FIREBALL.renderTexture(gWindow.getRenderer(), CLIENT.gProjectile[i].getPosX(), CLIENT.gProjectile[i].getPosY(), CLIENT.gProjectile[i].getAngle(), i, CLIENT_UNIQUE_ID, false, SDL_FLIP_NONE, EP.ANIM.FIREBALL_RENDERSPEED, 1, EP.EXECUTE.renderCollisionBox, CLIENT.gProjectile[i].getCollisionRect().w, CLIENT.gProjectile[i].getCollisionRect().h, CLIENT.gProjectile[i].getCollisionRect().x, CLIENT.gProjectile[i].getCollisionRect().y);
			}
		}
	}
	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
		{
			for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
			{
				if (!Player[i].gProjectile[j].getSlotFree())
				{
					ANIM_FIREBALL.renderTexture(gWindow.getRenderer(), Player[i].gProjectile[j].getPosX(), Player[i].gProjectile[j].getPosY(), Player[i].gProjectile[j].getAngle(), i, j, false, SDL_FLIP_NONE, EP.ANIM.FIREBALL_RENDERSPEED, 1, EP.EXECUTE.renderCollisionBox, CLIENT.gProjectile[i].getCollisionRect().w, CLIENT.gProjectile[i].getCollisionRect().h, CLIENT.gProjectile[i].getCollisionRect().x, CLIENT.gProjectile[i].getCollisionRect().y);
				}
			}
		}
	}

	//RENDER WALKING STATE

	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
		{
			if (Player[i].getAnimType() == "idle")
			{
				ANIM_IDLE.renderTexture(gWindow.getRenderer(), Player[i].getPosX(), Player[i].getPosY(), 0, i, 0,false, Player[i].getFlipType(),500,500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_IDLE.updateAnim(i, 0);
			}
			else if (Player[i].getAnimType() == "runAttack")
			{
				ANIM_RUNNING_ATTACK.renderTexture(gWindow.getRenderer(), Player[i].getPosX(), Player[i].getPosY(), 0, i, 0,false, Player[i].getFlipType(),500,500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_RUNNING_ATTACK.updateAnim(i, 0);
			}
			else if (Player[i].getAnimType() == "walking")
			{
				ANIM_WALKING.renderTexture(gWindow.getRenderer(), Player[i].getPosX(), Player[i].getPosY(), 0, i, 0,false, Player[i].getFlipType(),500,500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_WALKING.updateAnim(i, 0);
			}
			
			nickname_text_texture.loadFromRenderedText(Player[i].getNickname(), gColor, gWindow.getRenderer(), gNorthFont);
			nickname_text_texture.render(gWindow.getRenderer(), (Player[i].getPosX() + 35) - (strlen(Player[i].getNickname().c_str())), Player[i].getPosY(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
		}
	}


	//RENDER EXPOSION COLLISIONS

	ANIM_CONTACT_REDEXPLOSION.renderStaticAnim(gWindow.getRenderer(), EP.EXECUTE.renderCollisionBox,0,0,0,0);

	//RENDER CROSSHAIR

	crosshair_texture.render(gWindow.getRenderer(), mouseX - crosshair_texture.getWidth() / 2, mouseY - (crosshair_texture.getHeight() / 2) +40,NULL,NULL,NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

	//RENDER FPS COUNTER
	
	MEM.TEXTR.fpsText.render(gWindow.getRenderer(),0,0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

	//Render to window

	gWindow.render();

}

int sendPacket(void* ptr)
{
	stringstream fpsSS;
	int frame = 0;

	while (true)
	{
		if (EP.EXECUTE.isSendThreadActive)
		{
			SDL_Delay(SDL_GLOBAL_DELAY);

			EP.TEMP.DATAPACKET.clear();
			EP.TEMP.DATAPACKET.str(string());
			EP.TEMP.DATAPACKET << GET_DATA_ABOUT_PLAYER << "," << gServer.getClientID() << "," << gServer.getClientNickname() << "," << CLIENT.getPosX() << "," << CLIENT.getPosY() << ",";
			EP.TEMP.DATAPACKET << sFlipType << "," << CLIENT.getAnimType() << ",";

			if (EP.EXECUTE.injectProjectile)
			{
				EP.TEMP.DATAPACKET << UPDATE_BULLET << ",";
				EP.TEMP.DATAPACKET << EP.TEMP.projectileX << "," << EP.TEMP.projectileY << "," << EP.TEMP.projectileDX << "," << EP.TEMP.projectileDY << ",";
				EP.EXECUTE.injectProjectile = false;
			}

			EP.TEMP.DATAPACKET << END_OF_PACKET;

			clientSendData(EP.TEMP.DATAPACKET.str());
		}
		//SDL_Delay(SDL_GLOBAL_DELAY);
	}
	return 0;
}

bool init()
{
	bool success = true;

	int temp;
	string stemp;

	fstream gameSettings("data/gameSettings.txt", ios::in);
	//EP.FSTR.gameLog.open("data/gameLog.txt", ios::out | ios::app);

	gameSettings >> stemp >> temp;
	gWindow.setWidth(temp);
	gameSettings >> stemp >> temp;
	gWindow.setHeight(temp);
	gameSettings >> stemp >> temp;
	gWindow.setFullscreen(temp);

	gameSettings.close();

	ANIM_RUNNING_ATTACK.setTickTime(366);

	EP.TEMP.DATAPACKET_DEFAULT << END_OF_PACKET;

	EP.GSYS.exitLoopTimer.start();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		else
		{
			SDL_RenderSetLogicalSize(gWindow.getRenderer(), DEFAULT_RESOLUTION_WIDTH, DEFAULT_RESOLUTION_HEIGHT);

			typeLine_timer.start();

			user_textbox_button.setPosition(565,300,200,32);
			pass_textbox_button.setPosition(565,370,200,32);
			MEM.BTT.TWO_BUTTON.setPosition(350,300,290,80);
			MEM.BTT.FOUR_BUTTON.setPosition(650,300,290,80);

			register_button.setPosition(430,430,100,30);
			login_button.setPosition(760,435,100,30);

			/*

			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				{
					printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
					success = false;
				}
			}
			*/
			if (TTF_Init() == -1)
			{
				printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
				success = false;
			}

			if (!gWindow.init())
			{
				printf("Window 0 could not be created!\n");
				success = false;
			}
		}
	}
	return success;
}

bool connectToGameServer()
{
	result = NULL;
	ptr = NULL;
	ConnectSocket = INVALID_SOCKET;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("192.168.1.11", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << endl << "getaddrinfo failed:", iResult;
		WSACleanup();
		return false;
	}

	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Error at socket()" << WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		return false;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << endl << "Unable to connect to GAMESERVER!\n";
		WSACleanup();
		return false;
	}

	return true;
}

bool loadMedia()
{
	bool success = true;

	if (!loginPage_texture.loadFromFile("img/loginPage.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!green_textbox_texture.loadFromFile("img/textboxgreen.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!red_textbox_texture.loadFromFile("img/textboxred.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!background_texture.loadFromFile("img/background.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.PAWN_COLLISION_REFERENCE.loadFromFile("img/collision_reference.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!crosshair_texture.loadFromFile("img/crosshair.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!texture_brickFloor.loadFromFile("img/surfaces/floor.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.MATCHING_BUTTON_CHOICE.loadFromFile("img/matchingScreen/buttonUI.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.MATCHING_IN_PROGRESS.loadFromFile("img/matchingScreen/matching.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}

	//LOAD ANIMATIONS

	if (!ANIM_RUNNING.loadAnim(gWindow.getRenderer(), "img/mainChar/Running/0_Fallen_Angels_Running_000.png", 11))
	{
		cout << endl << "Failed to load anim 0_Fallen_Angels_Running_000.png";
	}
	if (!ANIM_WALKING.loadAnim(gWindow.getRenderer(), "img/mainChar/Walking/0_Fallen_Angels_Walking_000.png", 23))
	{
		cout << endl << "Failed to load anim 0_Fallen_Angels_Walking_000.png";
	}
	if (!ANIM_IDLE.loadAnim(gWindow.getRenderer(), "img/mainChar/Idle/0_Fallen_Angels_Idle_000.png", 17))
	{
		cout << endl << "Failed to load anim 0_Fallen_Angels_Idle_000.png";
	}
	if (!ANIM_RUNNING_ATTACK.loadAnim(gWindow.getRenderer(), "img/mainChar/Run_Slashing/0_Fallen_Angels_Run Slashing_000.png", 11))
	{
		cout << endl << "Failed to load anim Slashing_000.png";
	}
	if (!ANIM_FIREBALL.loadAnim(gWindow.getRenderer(), "img/attacks/fireball/fireball_000.png", 27))
	{
		cout << endl << "Failed to load anim fireball_000.png";
	}
	if (!ANIM_CONTACT_REDEXPLOSION.loadAnimCrop(gWindow.getRenderer(), "img/animation/red_explosion.png", 5, 5, 68, 68))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}


	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		Player[i].setMCWH(49, 65);
		Player[i].setCollisionRectWH(49, 65);
		Player[i].setCollisionOffset(25, 20);

		for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
		{
			Player[i].gProjectile[j].setMCWH(27, 27);
			Player[i].gProjectile[j].setCollisionRectWH(27, 27);
		}
	}
	for (unsigned int i = 0; i < 30; i++)
	{
		CLIENT.gProjectile[i].setMCWH(27, 27);
		CLIENT.gProjectile[i].setCollisionRectWH(27, 27);
	}

	CLIENT.setCollisionOffset(25,20);
	CLIENT.setCollisionRectWH(49, 65);
	CLIENT.setMCWH(49, 65);

	/*

	gMusic = Mix_LoadMUS("mix/spacemusic.wav");

	if (gMusic == NULL)
	{
		printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	bluebullet_sound = Mix_LoadWAV("mix/bluebullet_sound.wav");

	if (bluebullet_sound == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	if (!spaceback_texture.loadFromFile("img/spaceback.jpg",gRenderer))
	{
		printf("Failed to load press texture!\n");
		success = false;
	}
	if (!spaceback_texture.loadFromFile("img/spaceback.jpg", gRenderer))
	{
		printf("Failed to load press texture!\n");
		success = false;
	}

	*/

	gFont = TTF_OpenFont("font/lazy.ttf", MAX_PLAYER_BULLET_COUNT);
	gNorthFont = TTF_OpenFont("font/ArialCE.ttf", 12);
	MEM.FNT.gNorthFontLarge = TTF_OpenFont("font/ArialCE.ttf", 24);

	if (gFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	else
	{
		SDL_Color textColor = { 5, 255, 1 };

		if (!MEM.TEXTR.fpsText.loadFromRenderedText("FPS", gColor,gWindow.getRenderer(), gNorthFont))
		{
			printf("Failed to render text texture!\n");
			success = false;
		}
		/*
		if (!infotext_texture.loadFromRenderedText("Press enter to reset timer", gColor, gRenderer,gNorthFont))
		{
			printf("Failed to load press texture!\n");
			success = false;
		}
		if (!continueGame_text_texture.loadFromRenderedText("Continue Game",gWhite,gRenderer,gNorthFont))
		{
			printf("Failed to load up texture!\n");
			success = false;
		}
		if (!newGame_text_texture.loadFromRenderedText("New Game",gWhite,gRenderer,gNorthFont))
		{
			printf("Failed to load up texture!\n");
			success = false;
		}
		*/
	}

	return success;
}

void close()
{
	//health_bar_texture.free();

	EP.TEMP.DATAPACKET.clear();
	EP.TEMP.DATAPACKET.str(string());
	EP.TEMP.DATAPACKET << DELETE_PLAYER << "," << gServer.getClientID() << "," << gServer.getClientNickname() << "," << END_OF_PACKET;

	//iResult = send(ConnectSocket, EP.TEMP.DATAPACKET.str().c_str(), (int)strlen(EP.TEMP.DATAPACKET.str().c_str()), 0);

	Mix_FreeChunk(bluebullet_sound);
	Mix_FreeMusic(gMusic);
	gMusic = NULL;
	SDL_DestroyRenderer(gWindow.getRenderer());
	gWindow.free();

	stringstream temp;

	iResult = shutdown(ConnectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}

	WSACleanup();
	closesocket(ConnectSocket);

	Mix_Quit();
	IMG_Quit();
	SDL_Quit();

}

bool loginLoop()
{
	iResult = shutdown(ConnectSocket, SD_BOTH);
	closesocket(ConnectSocket);

	EP.EXECUTE.isReciveThreadActive = false;
	EP.EXECUTE.exitCurrentLoop = false;
	bool quit = false;
	bool inside = false;
	bool typeLinePos = false;
	int typeLineLenght = 0;
	bool newInput = false;
	bool attemptSuccesful = false;
	bool attemptFailed = false;
	bool alphaIn = true, alphaOut = false;
	bool loggedIn = false;

	int alpha = 55;

	SDL_StartTextInput();

	string user_ss = "admin";
	string pass_ss = "admin";
	string pStar;
	string text_info;

	while (EP.EXECUTE.exitCurrentLoop == false && loggedIn == false)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_TAB)
				{
					typeLinePos = !typeLinePos;
				}

				if (e.key.keysym.sym == SDLK_BACKSPACE && user_ss.length() > 0)
				{
					if (!typeLinePos)
					{
						user_ss.pop_back();
					}
				}
				if (e.key.keysym.sym == SDLK_BACKSPACE && pass_ss.length() > 0)
				{
					if (typeLinePos)
					{
						pass_ss.pop_back();
						pStar.pop_back();
					}
				}

				//Handle copy
				else if (e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL && !typeLinePos)
				{
					SDL_SetClipboardText(user_ss.c_str());
				}
				//Handle paste
				else if (e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL && !typeLinePos)
				{
					user_ss = SDL_GetClipboardText();
				}

				user_text_texture.loadFromRenderedText(user_ss.c_str(), gColor, gWindow.getRenderer(), gNorthFont);
				pass_text_texture.loadFromRenderedText(pStar.c_str(), gColor, gWindow.getRenderer(), gNorthFont);

			}
			else if (e.type == SDL_TEXTINPUT)
			{
				if (!((e.text.text[0] == 'c' || e.text.text[0] == 'C') && (e.text.text[0] == 'v' || e.text.text[0] == 'V') && SDL_GetModState() & KMOD_CTRL))
				{
					if (!typeLinePos)
					{
						user_ss += e.text.text;
					}
					else
					{
						pass_ss += e.text.text;
						pStar = pStar + '*';
					}
				}
				user_text_texture.loadFromRenderedText(user_ss.c_str(), gColor, gWindow.getRenderer(), gNorthFont);
				pass_text_texture.loadFromRenderedText(pStar.c_str(), gColor, gWindow.getRenderer(), gNorthFont);
			}

			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				if (user_textbox_button.handleClick(e))
				{
					typeLinePos = false;
				}
				if (pass_textbox_button.handleClick(e))
				{
					typeLinePos = true;
				}
				if (login_button.handleClick(e))
				{
					if (gServer.attemptLogin(user_ss, pass_ss))
					{
						attemptSuccesful = true;
						info_text_texture.loadFromRenderedText("Login succesful.", gColor, gWindow.getRenderer(), gNorthFont);
						loggedIn = true;
						popup_timer.start();
						connectInfo.clear();
						connectInfo.str(string());
						connectInfo << NEW_PLAYER << "," << gServer.getClientID() << "," << gServer.getClientNickname() << "," << END_OF_PACKET;
					}
					else
					{
						kickPlayerDuplicate = true;
						duplicateInfo.clear();
						duplicateInfo.str(string());
						duplicateInfo << DELETE_PLAYER << "," << gServer.getClientID() << "," << gServer.getClientNickname() << "," << END_OF_PACKET;
						attemptFailed = true;
						info_text_texture.loadFromRenderedText("Invalid Credentials.", gColor, gWindow.getRenderer(), gNorthFont);
						popup_timer.start();
					}
					//100 32
					user_ss = "";
					pass_ss = "";
					pStar = "";
				}
				if (register_button.handleClick(e))
				{
					if (gServer.attemptRegister(user_ss, pass_ss) == 0)
					{
						info_text_texture.loadFromRenderedText("Account created.", gColor, gWindow.getRenderer(), gNorthFont);
					}
					else if (gServer.attemptRegister(user_ss, pass_ss) == 1)
					{
						info_text_texture.loadFromRenderedText("Username already in use.", gColor, gWindow.getRenderer(), gNorthFont);
					}
					else if (gServer.attemptRegister(user_ss, pass_ss) == 2)
					{
						info_text_texture.loadFromRenderedText("Server Error.", gColor, gWindow.getRenderer(), gNorthFont);
					}
					else if (gServer.attemptRegister(user_ss, pass_ss) == 3)
					{
						info_text_texture.loadFromRenderedText("User/Pass not allowed.", gColor, gWindow.getRenderer(), gNorthFont);

					}

					attemptFailed = true;

					popup_timer.start();

					user_ss = "";
					pass_ss = "";
					pStar = "";
				}
			}

			else if (e.type == SDL_QUIT)
			{
				EP.EXECUTE.exitCurrentLoop = true;
			}

		}

		gWindow.handleEvent(e);
		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gWindow.getRenderer());

		background_texture.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

		user_text_texture.loadFromRenderedText(user_ss.c_str(), gColor, gWindow.getRenderer(), gNorthFont);
		pass_text_texture.loadFromRenderedText(pStar.c_str(), gColor, gWindow.getRenderer(), gNorthFont);

		loginPage_texture.render(gWindow.getRenderer(), gWindow.getWidth() * 0.5f - loginPage_texture.getWidth() / 2, gWindow.getHeight() * 0.5f - loginPage_texture.getHeight() / 2, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0, 0, 0, 0xFF);

		if (!typeLinePos)
		{
			typeLineLenght = user_ss.length() * 6.5f;
			if (typeLine_timer.getTicks() > 500)
			{
				if (typeLine_timer.getTicks() > 1000)
				{
					typeLine_timer.reset();
				}
				SDL_RenderDrawLine(gWindow.getRenderer(), gWindow.getWidth() * scaleX * 0.80f + typeLineLenght, gWindow.getHeight() * scaleX * 0.80f, gWindow.getWidth() * scaleX * 0.80f + typeLineLenght, gWindow.getHeight() * scaleX * 0.80f + 15);
			}
		}
		else
		{
			typeLineLenght = pass_ss.length() * 6.5f;
			if (typeLine_timer.getTicks() > 500)
			{
				if (typeLine_timer.getTicks() > 1000)
				{
					typeLine_timer.reset();
				}
				SDL_RenderDrawLine(gWindow.getRenderer(), gWindow.getWidth() * scaleX * 0.80f + typeLineLenght, gWindow.getHeight() * scaleX * 0.916f, gWindow.getWidth() * scaleX * 0.80f + typeLineLenght, gWindow.getHeight() * scaleX * 0.916f + 15);
			}
		}

		user_text_texture.render(gWindow.getRenderer(), gWindow.getWidth() * scaleX * 0.80f, gWindow.getHeight() * scaleX * 0.80f, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
		pass_text_texture.render(gWindow.getRenderer(), gWindow.getWidth() * scaleX * 0.80f, gWindow.getHeight() * scaleX * 0.916f, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

		if (attemptSuccesful)
		{
			if (popup_timer.getTicks() > 3000)
			{
				attemptSuccesful = false;
				popup_timer.stop();
				popup_timer.reset();
				alphaIn = true;
			}
			green_textbox_texture.setAlpha(alpha);
			green_textbox_texture.render(gWindow.getRenderer(), 555, 425, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

			info_text_texture.render(gWindow.getRenderer(), 575, 435, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

			loggedIn = true;

			return true;
		}
		if (attemptFailed)
		{
			if (popup_timer.getTicks() > 3000)
			{
				attemptFailed = false;
				popup_timer.stop();
				popup_timer.reset();
				alphaIn = true;
			}
			red_textbox_texture.setAlpha(alpha);
			red_textbox_texture.render(gWindow.getRenderer(), 555, 425, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);

			info_text_texture.render(gWindow.getRenderer(), 575, 435, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox,0,0,0,0);
		}

		gWindow.render();

		if (attemptFailed || attemptSuccesful)
		{
			if (alphaIn)
			{
				alpha = alpha + popup_timer.getTicks() / 550;
			}
			else
			{
				alpha = alpha - popup_timer.getTicks() / 550;
			}
			if (alpha > 255)
			{
				alphaIn = false;
			}
		}
		SDL_Delay(SDL_GLOBAL_DELAY);
	}

	SDL_StopTextInput();

	return false;
}

bool vulkanTest()
{
	while (true)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_KEYDOWN)
			{

			}
			else if (e.type == SDL_TEXTINPUT)
			{

			}

			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
	
			}

			else if (e.type == SDL_QUIT)
			{
				return false;
			}

		}

		gWindow.handleEvent(e);
		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gWindow.getRenderer());

		//texture_brickFloor.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
	
	//	VULKAN::createSurface(gWindow.getWindow(),instance)


		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0, 0, 0, 0xFF);
		gWindow.render();

	}

	return true;
}


string getData(int len, char* data)
{
	string sdata(data);
	sdata = sdata.substr(0, len);
	return sdata;
}

int oldSql(void* ptr)
{
	int frame = 0;

	LTimer fps;
	LTimer update;
	fps.start();
	update.start();

	while (bServerThread)
	{

	}
	return 0;
}

int v1, v2;

string getFinalData(const string & data)
{
	v2 = v1;
	v1 = data.find(',', v2 + 1);
	return data.substr(v2 + 1, v1 - v2 - 1);
}

int recivePacket(void* ptr)
{
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuff[DEFAULT_BUFLEN];
	
	string posX[MAX_PLAYER_ENTITY], posY[MAX_PLAYER_ENTITY], ID[MAX_PLAYER_ENTITY], nickname[MAX_PLAYER_ENTITY], count, rFlipType[MAX_PLAYER_ENTITY], animType[MAX_PLAYER_ENTITY];
	string posX2, posY2, id;
	bool exists = false;
	string data;
	int frame = 0, identifier;

	LTimer fps;
	LTimer update;
	fps.start();
	update.start();

	while (true)
	{
		if (EP.EXECUTE.isReciveThreadActive)
		{
			memset(recvbuff, 0, DEFAULT_BUFLEN);
			iResult = recv(ConnectSocket, recvbuff, DEFAULT_BUFLEN, 0);

			data = getData(iResult, recvbuff);
			v1 = data.find(',');
			identifier = atoi(data.substr(0, v1).c_str());

			if (identifier == GET_DATA_ABOUT_PLAYER)
			{
				count = getFinalData(data);
				for (unsigned int i = 0; i < atoi(count.c_str()); i++)
				{
					ID[i] = getFinalData(data);
					nickname[i] = getFinalData(data);
					posX[i] = getFinalData(data);
					posY[i] = getFinalData(data);
					rFlipType[i] = getFinalData(data);
					animType[i] = getFinalData(data);
				}

				for (unsigned int i = 0; i < atoi(count.c_str()); i++)
				{
					if (ID[i] != gServer.getClientID())
					{
						for (int j = 0; j < MAX_PLAYER_ENTITY; j++)
						{
							if (Player[j].getIfSlotUsed() && Player[j].getPlayerID() == ID[i])
							{
								Player[j].setPosX(atoi(posX[i].c_str()));
								Player[j].setPosY(atoi(posY[i].c_str()));
								Player[j].setFlipTypeString(rFlipType[i]);
								if (rFlipType[i] == "horizontal") Player[j].setFlipType(SDL_FLIP_HORIZONTAL);
								else Player[j].setFlipType(SDL_FLIP_NONE);

								Player[j].setAnimType(animType[i]);

								break;
							}
						}
					}
				}
			}
			else if (identifier == DELETE_PLAYER)
			{
				string ID = getFinalData(data);
				for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
				{
					if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead() && (Player[i].getPlayerID() == ID))
					{
						Player[i].setIfSlotUsed(false);
						cout << endl << "REMOVED PLAYER WITH ID " << ID;

						break;
					}
				}
			}
			else if (identifier == NEW_PLAYER)
			{
				/*
				ID[0] = getFinalData(data);
				nickname[0] = getFinalData(data);

				if (ID[0] != gServer.getClientID())
				{
					for (int j = 0; j < MAX_PLAYER_ENTITY; j++)
					{
						if (!Player[j].getIfSlotUsed())
						{
							cout << endl << "NEW PLAYER ON ID:" << j;
							Player[j].fotUsed(true);
							Player[j].setPlayerID(ID[0]);
							Player[j].setNickname(nickname[0]);

							break;
						}
					}
				}
				*/
			}
			else if (identifier == DAMAGE_PLAYER)
			{
				ID[0] = getFinalData(data); // dmg giver
				ID[1] = getFinalData(data); // dmg taker
				EP.TEMP.damageAmount = atoi(getFinalData(data).c_str());
				EP.TEMP.projIdentifier = atoi(getFinalData(data).c_str());

				for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
				{
					if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead() && Player[i].getID() == ID[1])
					{
						Player[i].damageTarget(EP.TEMP.damageAmount);

						if (EP.TEMP.projIdentifier == PROJ_KILLSHOT)
						{
							Player[i].setPlayerDead(true);
						}
						break;
					}
				}

				for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
				{
					if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
					{
						cout << endl << Player[i].getHealth();
					}
				}

			}
			else if (identifier == MATCHING_COMPLETE)
			{
				int mTypePC = atoi(getFinalData(data).c_str()) == TWO_PLAYER ? 2 : 4;

				for (int i = 0; i < mTypePC; i++)
				{
					ID[i] = getFinalData(data);
					nickname[i] = getFinalData(data);

					if (ID[i] != gServer.getClientID())
					{
						for (int j = 0; j < MAX_PLAYER_ENTITY; j++)
						{
							if (!Player[j].getIfSlotUsed())
							{
								Player[j].setIfSlotUsed(true);
								Player[j].setPlayerID(ID[i]);
								Player[j].setNickname(nickname[i]);
								cout << endl << "NEW PLAYER ON ID:" << j << " MAX:" << mTypePC;

								break;
							}
						}
					}
				}
				EP.EXECUTE.inMatchingScreen = false;
			}
			else if (identifier == UPDATE_BULLET)
			{
				ID[0] = getFinalData(data);
				nickname[0] = getFinalData(data);
				posX[0] = getFinalData(data);
				posY[0] = getFinalData(data);
				posX2 = getFinalData(data);
				posY2 = getFinalData(data);

				for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
				{
					if (ID[0] == Player[i].getPlayerID() && ID[0] != gServer.getClientID())
					{
						for (int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
						{
							if (Player[i].gProjectile[j].getSlotFree())
							{
								Player[i].gProjectile[j].setSlotFree(false);
								Player[i].gProjectile[j].setPosX(atoi(posX[0].c_str()));
								Player[i].gProjectile[j].setPosY(atoi(posY[0].c_str()));
								Player[i].gProjectile[j].setDestX(atoi(posX2.c_str()));
								Player[i].gProjectile[j].setDestY(atoi(posY2.c_str()));
								Player[i].gProjectile[j].setAngle(90 + (atan2(atoi(posY2.c_str()) - atoi(posY[0].c_str()), atoi(posX2.c_str()) - atoi(posX[0].c_str())) * 180 / 3.14f));
								Player[i].gProjectile[j].setVelX((atoi(posX2.c_str()) - atoi(posX[0].c_str())) / 50.0f);
								Player[i].gProjectile[j].setVelY((atoi(posY2.c_str()) - atoi(posY[0].c_str())) / 50.0f);
								break;
							}
						}
						break;
					}
				}
			}
			else if (identifier == MATCH_RESULT)
			{
				ID[0] = getFinalData(data);

				if (ID[0] == gServer.getClientID())
				{
					cout << endl << "WON MATCH :";
					resetPlayerData();
					tryLoopExit();
				}
				else
				{
					cout << endl << "LOST MATCH, WINNER ID:" << ID[0];
					resetPlayerData();
					tryLoopExit();
				}
			}
			else if (identifier == SET_POSITION)
			{
				posX[0] = getFinalData(data);
				posY[0] = getFinalData(data);

				CLIENT.setPosX(atoi(posX[0].c_str()));
				CLIENT.setPosY(atoi(posY[0].c_str()));
			}
			else if (identifier == KILL_PLAYER)
			{
				ID[0] = getFinalData(data); // dmg giver
				ID[1] = getFinalData(data); // dmg taker

				for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
				{
					if (Player[i].getIfSlotUsed() && Player[i].getID() == ID[1])
					{
						Player[i].setPlayerDead(true);

						cout << endl << id[0] << " killed " << ID[1];

						break;
					}
				}
			}
		}
	}
}

void computeFPS()
{
	//FPS COUNTER

	frame++;

	if (EP.GSYS.fpsTimer.getTicks() > 444)
	{
		//Calculate the frames per second and create the string

		fpsSS.clear();
		fpsSS.str("");
		fpsSS << "FPS:" << (int)(frame / (EP.GSYS.fpsTimer.getTicks() / 1000.f));

		MEM.TEXTR.fpsText.loadFromRenderedText(fpsSS.str(), gColor, gWindow.getRenderer(), MEM.FNT.gNorthFontLarge);

		//Restart the update timer
		EP.GSYS.fpsTimer.start();
		frame = 0;
	}
}

bool matchingLoop()
{
	SDL_ShowCursor(true);

	SDL_DetachThread(THREAD.PHYSICS);
	SDL_DetachThread(THREAD.SEND_DATA);

	SDL_Delay(10);

	EP.EXECUTE.isMatching = false;
	EP.EXECUTE.inMatchingScreen = true;
	EP.EXECUTE.exitCurrentLoop = false;
	resetPlayerData();

	EP.TEMP.DATAPACKET.clear();
	EP.TEMP.DATAPACKET.str(string());
	EP.TEMP.DATAPACKET << START_MATCHMAKING << "," << gServer.getClientID() << "," << gServer.getClientNickname() << ",";

	if (!connectToGameServer())
	{
		cout << endl << "[FAILED TO CONNECT TO GAME SERVER] " << WSAGetLastError();
		return false;
	}
	else
	{
		cout << endl << "[CONNECTED TO GAME SERVER]";
	}

	clientSendData(connectInfo.str());

	while (EP.EXECUTE.inMatchingScreen && !EP.EXECUTE.exitCurrentLoop)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				EP.EXECUTE.exitCurrentLoop = true;
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				mouseX = e.motion.x;
				mouseY = e.motion.y;
			}
			else if(e.type == SDL_MOUSEBUTTONDOWN)
			{
				//cout << endl << mouseX << " " << mouseY;
				if (!EP.EXECUTE.isMatching)
				{
					if (MEM.BTT.TWO_BUTTON.handleClick(e))
					{
						EP.EXECUTE.isMatching = true;
						EP.TEMP.matchingType = TWO_PLAYER;

						EP.TEMP.DATAPACKET << EP.TEMP.matchingType << "," << END_OF_PACKET;

						cout << endl << "SENDING:" << EP.TEMP.DATAPACKET.str();

						clientSendData(EP.TEMP.DATAPACKET.str());
						
					}
					else if (MEM.BTT.FOUR_BUTTON.handleClick(e))
					{
						EP.EXECUTE.isMatching = true;
						EP.TEMP.matchingType = FOUR_PLAYER;

						EP.TEMP.DATAPACKET << EP.TEMP.matchingType << "," << END_OF_PACKET;

						clientSendData(EP.TEMP.DATAPACKET.str());
					}
				}
			}
		}

		SDL_RenderClear(gWindow.getRenderer());
		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0, 0, 0, 0xFF);

		if (!EP.EXECUTE.isMatching)
		{
			background_texture.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
			MEM.TEXTR.MATCHING_BUTTON_CHOICE.render(gWindow.getRenderer(), gWindow.getWidth() / 2 - MEM.TEXTR.MATCHING_BUTTON_CHOICE.getWidth() / 2, gWindow.getHeight() / 2 - MEM.TEXTR.MATCHING_BUTTON_CHOICE.getHeight() / 2 - 20, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
		}
		else
		{
			background_texture.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
			MEM.TEXTR.MATCHING_IN_PROGRESS.render(gWindow.getRenderer(), gWindow.getWidth() / 2 - MEM.TEXTR.MATCHING_IN_PROGRESS.getWidth() / 2, gWindow.getHeight() / 2 - MEM.TEXTR.MATCHING_IN_PROGRESS.getHeight() / 2, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
		}

		clientSendData(EP.TEMP.DATAPACKET_DEFAULT.str());

		gWindow.handleEvent(e);
		gWindow.render();

		SDL_Delay(FPS_LIMIT_DELAY);
	}

	if (EP.EXECUTE.exitCurrentLoop == true)
	{
		return false;
	}
	else
	{
		EP.EXECUTE.isMatching = false;
		return true;
	}

}

bool playLoop()
{
	EP.EXECUTE.exitCurrentLoop = false;
	bool quit = false;
	bool inside = false;
	int xLast = 0, yLast = 0;

	bool collisionFound = false;
	
	fireball_attack_timer.start();

	SDL_ShowCursor(false);

	EP.GSYS.physicsTimer.start();
	EP.GSYS.physicsTimerMovement.start();
	EP.GSYS.fpsTimer.start();

	while (EP.EXECUTE.exitCurrentLoop == false)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_SPACE)
				{

				}
				if (e.key.keysym.sym == SDLK_MINUS)
				{
					if (EP.EXECUTE.renderCollisionBox == true)
					{
						EP.EXECUTE.renderCollisionBox = false;
					}
					else
					{
						EP.EXECUTE.renderCollisionBox = true;
					}
				}
			}
			else if (e.type == SDL_TEXTINPUT)
			{
				if (!((e.text.text[0] == 'c' || e.text.text[0] == 'C') && (e.text.text[0] == 'v' || e.text.text[0] == 'V') && SDL_GetModState() & KMOD_CTRL))
				{

				}
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (e.button.button == SDL_BUTTON_LEFT && fireball_attack_timer.getTicks() > 500)
				{
					fireball_attack_timer.reset();

					ANIM_RUNNING_ATTACK.setInUse(true);
					CLIENT.spawnProjectile(CLIENT.getPosX(), CLIENT.getPosY(), 0, e.button.x, e.button.y, 50.0f);
					CLIENT.setProjectileActive(true);

					EP.EXECUTE.injectProjectile = true;

					EP.TEMP.projectileX = CLIENT.getPosX();
					EP.TEMP.projectileY = CLIENT.getPosY();
					EP.TEMP.projectileDX = e.button.x;
					EP.TEMP.projectileDY = e.button.y;
				}

			}
			else if (e.type == SDL_QUIT)
			{
				EP.EXECUTE.exitCurrentLoop = true;
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				mouseX = e.motion.x;
				mouseY = e.motion.y;
			}
		}

		gWindow.handleEvent(e);

		//CHECK FOR NEW PROJ COLLISIONS

		if (addNewCollisionAnim)
		{
			for (unsigned int i = 0; i < MAX_PLAYER_BULLET_COUNT; i++)
			{
				if (!ANIM_CONTACT_REDEXPLOSION.getCropInUse(i))
				{
					ANIM_CONTACT_REDEXPLOSION.setCropInUse(i, true);
					ANIM_CONTACT_REDEXPLOSION.setCropPosX(i, animCollisionX);
					ANIM_CONTACT_REDEXPLOSION.setCropPosY(i, animCollisionY);

					addNewCollisionAnim = false;

					break;
				}
			}
		}

		computeFPS();

		renderTextures();

		SDL_Delay(FPS_LIMIT_DELAY);
	}

	EP.EXECUTE.isSendThreadActive = false;
	EP.EXECUTE.isPhysicsThreadActive = false;

	iResult = shutdown(ConnectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}

	closesocket(ConnectSocket);

	return false;
}

//OTHER GAMES BREAK WHEN AN ONGOING GAME IS TERMINATED

int main(int argc, char* args[])
{
	if (!init())
	{	
		printf("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			srand(time(NULL)); 

			/*

			THREAD.recvThread = SDL_CreateThread(recivePacket, "SendPacket", (void*)NULL);
			THREAD.SEND_DATA = SDL_CreateThread(sendPacket, "SendPacket", (void*)NULL);
			THREAD.PHYSICS = SDL_CreateThread(processPhysics, "processPhysics", (void*)NULL);

			while (loginLoop())
			{
				EP.EXECUTE.isReciveThreadActive = true;
				while (matchingLoop())
				{
					EP.EXECUTE.isSendThreadActive = true;
					EP.EXECUTE.isPhysicsThreadActive = true;

					while (playLoop())
					{
						
					}

				}
			}
			*/

			initVulkan();

			vulkanTest();
		}
	}
	close();

	return 0;
}