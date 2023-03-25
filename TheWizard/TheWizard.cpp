// TheWizard.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <winsock2.h>
//#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include <fstream>
#include <conio.h>
#include "SDL.h"
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
#include "LMap.h"
#include "LCrypto.h"
#include <SDL_vulkan.h>
#include <vulkan.h>
#include <GLSL.std.450.h>
#include "Shader.h"
#include <functional>
#include <deque>
#include "Vulkan.h"

WSADATA wData;
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 10000
#define MAX_PLAYER_ENTITY 4
#define MAX_PLAYER_BULLET_COUNT 28
#define SDL_GLOBAL_DELAY 10
#define FPS_LIMIT_DELAY 10
#define CLIENT_UNIQUE_ID 29
#define MAX_MAPS 10

using namespace std;

#define DEFAULT_RESOLUTION_WIDTH 1920	
#define DEFAULT_RESOLUTION_HEIGHT 1080

#define INITIAL_RESOLUTION_WIDTH 1280
#define INITIAL_RESOLUTION_HEIGHT 720	

struct engineThreads
{
	SDL_Thread* RECIVE_DATA = NULL;
	SDL_Thread* SEND_DATA = NULL;
	SDL_Thread* PHYSICS = NULL;

}THREAD;

struct MEMEORY
{
	struct TEXTURES
	{
		LTexture miscText;
		LTexture fpsText;
		LTexture PAWN_COLLISION_REFERENCE;
		LTexture MATCHING_BUTTON_CHOICE;
		LTexture MATCHING_IN_PROGRESS;
		LTexture MATCH_RESULT_WON;
		LTexture MATCH_RESULT_LOST;
		LTexture GRASS_BG;
		LTexture BRICK_FLOOR;
		LTexture NEW_OBJECT;


	}TEXTR;
	struct FONT
	{
		TTF_Font* gNorthFontLarge = NULL;
	}FNT;
	struct BUTTON
	{
		LButton TWO_BUTTON;
		LButton FOUR_BUTTON;
		LButton LOGIN;
		LButton REGISTER;
		LButton MATCHING_2PLAYER;
		LButton MATCHING_4PLAYER;

	}BTT;
	struct MAPS
	{
		LMap* CURRENT_MAP = new LMap;
		LMap GRASS_WORLD;
	}MAP;
	struct OBJECT_DATA
	{
		LCrypto Crypto;

	}OBJ;
}MEM;

struct MAPLIST
{
	string MAP_NAME[MAX_MAPS];
	SDL_Point MAP_SIZE[MAX_MAPS];

}MAPL;

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
	SET_POSITION,
	ENCRYPTION_INFO
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

bool SKIP_CONN = true;

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

Mix_Music* gMusic = NULL;
Mix_Chunk* bluebullet_sound = NULL;
TTF_Font* gFont = NULL;
TTF_Font* gNorthFont = NULL;
SDL_Event e;

LServer gServer;
LWindow gWindow(DEFAULT_RESOLUTION_WIDTH, DEFAULT_RESOLUTION_HEIGHT);
Vulkan VK(gWindow);

LPawn CLIENT;
LPawn Player[MAX_PLAYER_ENTITY];

SDL_Color gColor = { 255,0,0 };

LTexture loginPage_texture;
LTexture user_text_texture;
LTexture pass_text_texture;
LTexture green_textbox_texture;
LTexture red_textbox_texture;
LTexture info_text_texture;
LTexture background_texture;
LTexture crosshair_texture;
LTexture nickname_text_texture;
LTexture sTexture;

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
struct engineParameters
{
	struct FSTREAM
	{
		fstream gameLog;
		fstream mapData;
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
		LTimer matchResultTimer;
		int physicsRate = 5;
		float DEFAULT_PROJ_SPEED = 15.0f;
		SDL_Rect RESOLUTION_CLIP;

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
		bool MATCH_RESULT_SCREEN = false;
		bool INSERT_OBJECT = false;

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
		bool MATCH_RESULT_WON = false;
		int CAMERA_X;
		int CAMERA_Y;
		int MAP_OBJECT_ID;
		int MOUSE_X;
		int MOUSE_Y;

		stringstream miscSS;
		stringstream DATAPACKET;
		stringstream DATAPACKET_DEFAULT;

		SDL_Rect OBJECT_RECT;
		VkResult RESULT_VK;


	}TEMP;
	struct RENDERING
	{
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
		VkCommandBuffer COMMAND_BUFFER_VK  = nullptr;
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


	}RND;

}EP;


bool initVulkan();
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
void matchResultScreen();
void testEnviroment();
VkInstance createVkInstance(SDL_Window* window);
bool isDeviceSuitable(VkPhysicalDevice device);
VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void recordRenderCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);

void vkRender()
{

	

	vkAcquireNextImageKHR(VK.getLogicalDevice(), VK.getSwapchain(), UINT64_MAX, VK.getSemaphoreAvailable(), VK_NULL_HANDLE, VK.getImageIndex());

	VK.getRenderPassBeginInfo()->framebuffer = VK.getSwapchainFramebuffer(*VK.getImageIndex());

	vkQueueSubmit(VK.getGraphicsQueue(), 1, VK.getSubmitInfo(), VK_NULL_HANDLE);

	vkWaitForFences(VK.getLogicalDevice(), 1, VK.getFenceRenderingFinished(), VK_TRUE, UINT64_MAX);

	vkBeginCommandBuffer(VK.getCommandBuffer(), VK.getCommandBufferBeginInfo());

	vkCmdBeginRenderPass(VK.getCommandBuffer(), VK.getRenderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

	//Rendering commands
	vkCmdBindPipeline(VK.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, VK.getGraphicsPipeline());
	

	vkCmdDraw(VK.getCommandBuffer(), 3, 1, 0, 0);

	vkQueuePresentKHR(VK.getGraphicsQueue(), VK.getPresentInfo());

	vkCmdEndRenderPass(VK.getCommandBuffer());

	vkEndCommandBuffer(VK.getCommandBuffer());

	vkResetFences(VK.getLogicalDevice(), 1, VK.getFenceRenderingFinished());

}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(EP.RND.PHYSICAL_DEVICE_VK, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

bool isDeviceSuitable(VkPhysicalDevice device) 
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) 
{
	
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	cout << endl << createInfo.pCode << "-code";
	VkShaderModule shaderModule  = VK_NULL_HANDLE;

	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}

VkMemoryPropertyFlags getRequiredMemoryFlags(VkBufferUsageFlags usageFlags) 
{
	VkMemoryPropertyFlags requiredFlags = 0;

	if (usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

	if (usageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

	if (usageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

	// add other usage flags as needed

	return requiredFlags;
}

std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return {};
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

bool initVulkan()
{
	EP.RND.INSTANCE_VK = createVkInstance(gWindow.getWindow());

	if (EP.RND.INSTANCE_VK == VK_NULL_HANDLE)
	{
		cout << endl << "Instance creation failed";
		return false;
	}

	if (!SDL_Vulkan_CreateSurface(gWindow.getWindow(), EP.RND.INSTANCE_VK, &EP.RND.SURFACE_VK))
	{
		cout << endl << "Surface creation failed";
		return false;
	}

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(EP.RND.INSTANCE_VK, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(EP.RND.INSTANCE_VK, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			EP.RND.PHYSICAL_DEVICE_VK = device;
			break;
		}
	}


	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(EP.RND.PHYSICAL_DEVICE_VK, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(EP.RND.PHYSICAL_DEVICE_VK, &queueFamilyCount, queueFamilies.data());

	// Find a queue family that supports the required capabilities

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			EP.RND.GRAPHICS_QUEUE_FAMILY_INDEX_VK = i;
			break;
		}
	}

	if (EP.RND.GRAPHICS_QUEUE_FAMILY_INDEX_VK == UINT32_MAX) {
		// Error: no queue family found that supports the required capabilities
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = EP.RND.GRAPHICS_QUEUE_FAMILY_INDEX_VK;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;


	std::vector<const char*> deviceExtensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;


	vkGetPhysicalDeviceProperties(EP.RND.PHYSICAL_DEVICE_VK, &EP.RND.DEVICE_PROPERTIES_VK);
	vkGetPhysicalDeviceFeatures(EP.RND.PHYSICAL_DEVICE_VK, &EP.RND.DEVICE_FEATURES_VK);


	//create a logical device using parameters : deviceFeatures,EP.RND.PHYSICAL_DEVICE_VK

	EP.RND.RESULT_VK = vkCreateDevice(EP.RND.PHYSICAL_DEVICE_VK, &deviceCreateInfo, nullptr, &EP.RND.LOGICAL_DEVICE_VK);

	if (EP.RND.RESULT_VK != VK_SUCCESS) {
		// Handle error - device creation failed
		switch (EP.RND.RESULT_VK)
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			// Handle out-of-host-memory error
			cout << endl << "VK_ERROR_OUT_OF_HOST_MEMORY";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			// Handle out-of-device-memory error
			cout << endl << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			// Handle initialization failed error
			cout << endl << "VK_ERROR_INITIALIZATION_FAILED";
			break;
		default:
			// Handle unknown error code
			cout << endl << "VK_ERROR_UNKNOWN";
			break;
		}
	}

	// Query available surface formats
	uint32_t formatCount;

	vkGetPhysicalDeviceSurfaceFormatsKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &formatCount, formats.data());

	// Choose a surface format
	VkSurfaceFormatKHR surfaceFormat;
	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		// If the surface has no preferred format, choose a default one
		surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	else 
	{
		// Otherwise, select the first available format that meets our requirements
		for (const auto& format : formats) 
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				surfaceFormat = format;
				break;
			}
		}
	}

	// Query available present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &presentModeCount, presentModes.data());

	// Choose a present mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Default to FIFO mode
	for (const auto& mode : presentModes) 
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = mode;
			break;
		}
		else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			presentMode = mode;
		}
	}

	// Create a VkSwapchainKHR 
	// Choose the extent of the swapchain images
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &surfaceCapabilities);


	if (surfaceCapabilities.currentExtent.width == UINT32_MAX) 
	{
		// If the surface size is undefined, set the size to the requested width and height
		EP.RND.SWAPCHAIN_EXTENT.width = gWindow.getWidth();
		EP.RND.SWAPCHAIN_EXTENT.height = gWindow.getHeight();
	}
	else {
		// Otherwise, use the surface size as the swapchain size
		EP.RND.SWAPCHAIN_EXTENT = surfaceCapabilities.currentExtent;
	}

	// Determine the number of swapchain images
	uint32_t desiredSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && desiredSwapchainImages > surfaceCapabilities.maxImageCount) {
		desiredSwapchainImages = surfaceCapabilities.maxImageCount;
	}


	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &surfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(EP.RND.PHYSICAL_DEVICE_VK, EP.RND.SURFACE_VK, &surfaceFormatCount, surfaceFormats.data());

	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		for (const auto& availableFormat : surfaceFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = availableFormat;
				break;
			}
		}
		// If the desired format is not available, choose the first available format
		if (surfaceFormats.size() > 0) {
			surfaceFormat = surfaceFormats[0];
		}
	}

	// Fill in the swapchain creation info
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = EP.RND.SURFACE_VK;
	swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = EP.RND.SWAPCHAIN_EXTENT;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create the swapchain

	if (EP.RND.LOGICAL_DEVICE_VK == NULL || &swapchainCreateInfo == NULL || &EP.RND.SWAPCHAIN_VK == NULL)
	{
		cout << endl << "FAILED";
	}

	VkResult result = VK_SUCCESS;
	result = vkCreateSwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, &swapchainCreateInfo, nullptr, &EP.RND.SWAPCHAIN_VK);

	if (result != VK_SUCCESS) 
	{
		// Error handling
		cout << "SWAP CHAIN FAILED";
		return false;
	}
	// Get the swapchain images

	uint32_t swapchainImageCount;
	result = vkGetSwapchainImagesKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, &swapchainImageCount, nullptr);
	if (result != VK_SUCCESS) {
		// Handle swapchain image retrieval error
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		return result;
	}
	// Allocate memory for the swapchain images

	std::vector<VkImage> swapchainImages(swapchainImageCount);
	result = vkGetSwapchainImagesKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, &swapchainImageCount, swapchainImages.data());
	if (result != VK_SUCCESS) {
		// Handle swapchain image retrieval error
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		return result;
	}

	// Create an image view for each swapchain image
	std::vector<VkImageView> swapchainImageViews(swapchainImageCount);
	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = surfaceFormat.format;
		imageViewCreateInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY, // R
			VK_COMPONENT_SWIZZLE_IDENTITY, // G
			VK_COMPONENT_SWIZZLE_IDENTITY, // B
			VK_COMPONENT_SWIZZLE_IDENTITY  // A
		};
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(EP.RND.LOGICAL_DEVICE_VK, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
		if (result != VK_SUCCESS) {
			// Handle image view creation error
			vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
			return result;
		}
	}
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	
	EP.RND.RENDER_PASS_INFO_VK.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	EP.RND.RENDER_PASS_INFO_VK.attachmentCount = 1;
	EP.RND.RENDER_PASS_INFO_VK.pAttachments = &colorAttachment;
	EP.RND.RENDER_PASS_INFO_VK.subpassCount = 1;
	EP.RND.RENDER_PASS_INFO_VK.pSubpasses = &subpass;

	result = vkCreateRenderPass(EP.RND.LOGICAL_DEVICE_VK, &EP.RND.RENDER_PASS_INFO_VK, nullptr, &EP.RND.RENDER_PASS_VK);

	if (result != VK_SUCCESS) 
	{
		// Handle render pass creation error
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		cout << "RENDER PASS FAILED";
		return result;
	}
	cout << endl << swapchainImageCount;
	EP.RND.SWAPCHAIN_FRAMEBUFFER_VK.assign(swapchainImageCount, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{

		VkImageView attachments[] = {swapchainImageViews[i]};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = EP.RND.RENDER_PASS_VK;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = gWindow.getWidth();
		framebufferCreateInfo.height = gWindow.getHeight();
		framebufferCreateInfo.layers = 1;

		result = vkCreateFramebuffer(EP.RND.LOGICAL_DEVICE_VK, &framebufferCreateInfo, nullptr, &EP.RND.SWAPCHAIN_FRAMEBUFFER_VK[i]);
		if (result != VK_SUCCESS) {
			// Handle framebuffer creation error
			for (uint32_t j = 0; j < i; j++) {
				//vkDestroyFramebuffer(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_FRAMEBUFFER_VK[j], nullptr);
			}
			vkDestroyRenderPass(EP.RND.LOGICAL_DEVICE_VK, EP.RND.RENDER_PASS_VK, nullptr);
			vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
			return result;
		}
	}

	// Create a descriptor set layout for the uniform values
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = 1;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &layoutBinding;

	VkDescriptorSetLayout descriptorSetLayout;
	result = vkCreateDescriptorSetLayout(EP.RND.LOGICAL_DEVICE_VK, &layoutCreateInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS) {
		// Handle descriptor set layout creation error
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		return result;
	}

	// Create the pipeline layout using the descriptor set layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

	VkPipelineLayout pipelineLayout;
	result = vkCreatePipelineLayout(EP.RND.LOGICAL_DEVICE_VK, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS) {
		// Handle pipeline layout creation error
		vkDestroyDescriptorSetLayout(EP.RND.LOGICAL_DEVICE_VK, descriptorSetLayout, nullptr);
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		return result;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0; // Specify the number of vertex binding descriptions
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Pointer to an array of vertex binding descriptions
	vertexInputInfo.vertexAttributeDescriptionCount = 0; // Specify the number of vertex attribute descriptions
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Pointer to an array of vertex attribute descriptions 

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)gWindow.getWidth();
	viewport.height = (float)gWindow.getHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = surfaceCapabilities.currentExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling configuration
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Depth-stencil configuration
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	// Color blending configuration
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	EP.RND.VERT_SHADER_MODULE = createShaderModule(EP.RND.LOGICAL_DEVICE_VK, readFile("shaders/colored_triangle_vertex.spv"));
	EP.RND.FRAG_SHADER_MODULE = createShaderModule(EP.RND.LOGICAL_DEVICE_VK, readFile("shaders/colored_triangle_frag.spv"));

	// Define shader stage create info structures
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = EP.RND.VERT_SHADER_MODULE;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = EP.RND.FRAG_SHADER_MODULE;
	fragShaderStageInfo.pName = "main";

	// Create array of shader stage create info structures

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencil;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = EP.RND.RENDER_PASS_VK;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;


	//Create pipeline 1

	EP.TEMP.RESULT_VK = vkCreateGraphicsPipelines(EP.RND.LOGICAL_DEVICE_VK, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &EP.RND.GRAPHICS_PIPELINE_VK);
	if (EP.TEMP.RESULT_VK != VK_SUCCESS) {
		// Handle pipeline creation error
		vkDestroyPipelineLayout(EP.RND.LOGICAL_DEVICE_VK, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(EP.RND.LOGICAL_DEVICE_VK, descriptorSetLayout, nullptr);
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		return EP.TEMP.RESULT_VK;
	}


	EP.RND.VERT_SHADER_MODULE = createShaderModule(EP.RND.LOGICAL_DEVICE_VK, readFile("shaders/vertex_shader3.spv"));
	EP.RND.FRAG_SHADER_MODULE = createShaderModule(EP.RND.LOGICAL_DEVICE_VK, readFile("shaders/fs.spv"));

	// Define shader stage create info structures
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = EP.RND.VERT_SHADER_MODULE;
	vertShaderStageInfo.pName = "main";

	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = EP.RND.FRAG_SHADER_MODULE;
	fragShaderStageInfo.pName = "main";

	// Create array of shader stage create info structures

	VkPipelineShaderStageCreateInfo shaderStages2[] = { vertShaderStageInfo, fragShaderStageInfo };

	pipelineCreateInfo = {};

	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages2;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencil;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = EP.RND.RENDER_PASS_VK;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	//Create pipeline 2

	EP.TEMP.RESULT_VK = vkCreateGraphicsPipelines(EP.RND.LOGICAL_DEVICE_VK, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &EP.RND.GRAPHICS_PIPELINE_2_VK);
	if (EP.TEMP.RESULT_VK != VK_SUCCESS) {
		// Handle pipeline creation error
		vkDestroyPipelineLayout(EP.RND.LOGICAL_DEVICE_VK, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(EP.RND.LOGICAL_DEVICE_VK, descriptorSetLayout, nullptr);
		vkDestroySwapchainKHR(EP.RND.LOGICAL_DEVICE_VK, EP.RND.SWAPCHAIN_VK, nullptr);
		return EP.TEMP.RESULT_VK;
	}

	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = queueCreateInfo.queueFamilyIndex;

	if (vkCreateCommandPool(EP.RND.LOGICAL_DEVICE_VK, &poolCreateInfo, nullptr, &EP.RND.COMMAND_POOL_VK) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create command pool");
	}

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = EP.RND.COMMAND_POOL_VK;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(EP.RND.LOGICAL_DEVICE_VK, &allocateInfo, &EP.RND.COMMAND_BUFFER_VK) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer");
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(EP.RND.LOGICAL_DEVICE_VK, &semaphoreCreateInfo, nullptr, &EP.RND.SEMAPHORE_IMAGE_AVAILABLE_VK) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create semaphore for image availability!");
	}

	EP.TEMP.RESULT_VK = vkCreateSemaphore(EP.RND.LOGICAL_DEVICE_VK, &semaphoreCreateInfo, nullptr, &EP.RND.SEMAPHORE_RENDERING_FINISHED_VK);
	if (EP.TEMP.RESULT_VK != VK_SUCCESS) {
		// Handle semaphore creation error
		cout << "Failed to create semaphore for rendering" << endl;
	}

	vkGetDeviceQueue(EP.RND.LOGICAL_DEVICE_VK, EP.RND.GRAPHICS_QUEUE_FAMILY_INDEX_VK, 0, &EP.RND.GRAPHICS_QUEUE_VK);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	EP.TEMP.RESULT_VK = vkCreateFence(EP.RND.LOGICAL_DEVICE_VK, &fenceCreateInfo, nullptr, &EP.RND.FENCE_RENDERING_FINISHED_VK);
	if (EP.TEMP.RESULT_VK != VK_SUCCESS) {
		// Handle fence creation error
		cout << "Failed to create fence for rendering" << endl;
	}


	EP.RND.PRESENT_INFO_VK.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	EP.RND.PRESENT_INFO_VK.waitSemaphoreCount = 1;
	EP.RND.PRESENT_INFO_VK.pWaitSemaphores = &EP.RND.SEMAPHORE_IMAGE_AVAILABLE_VK;
	EP.RND.PRESENT_INFO_VK.swapchainCount = 1;
	EP.RND.PRESENT_INFO_VK.pSwapchains = &EP.RND.SWAPCHAIN_VK;
	EP.RND.PRESENT_INFO_VK.pImageIndices = &EP.RND.IMAGE_INDEX_VK;

	// Record commands to the command buffer

	EP.RND.COMMAND_BUFFER_BEGIN_INFO_VK.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	EP.RND.COMMAND_BUFFER_BEGIN_INFO_VK.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;


	EP.RND.RENDER_PASS_BEGIN_INFO_VK.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	EP.RND.RENDER_PASS_BEGIN_INFO_VK.renderPass = EP.RND.RENDER_PASS_VK;
	EP.RND.RENDER_PASS_BEGIN_INFO_VK.renderArea.offset = { 0, 0 };
	EP.RND.RENDER_PASS_BEGIN_INFO_VK.renderArea.extent = EP.RND.SWAPCHAIN_EXTENT;
	EP.RND.RENDER_PASS_BEGIN_INFO_VK.clearValueCount = 1;
	EP.RND.RENDER_PASS_BEGIN_INFO_VK.pClearValues = &EP.RND.WINDOW_RENDER_COLOR_VK;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	EP.RND.SUBMIT_INFO_VK.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	EP.RND.SUBMIT_INFO_VK.waitSemaphoreCount = 1;
	EP.RND.SUBMIT_INFO_VK.pWaitSemaphores = &EP.RND.SEMAPHORE_IMAGE_AVAILABLE_VK;
	EP.RND.SUBMIT_INFO_VK.pWaitDstStageMask = waitStages;
	EP.RND.SUBMIT_INFO_VK.commandBufferCount = 1;
	EP.RND.SUBMIT_INFO_VK.pCommandBuffers = &EP.RND.COMMAND_BUFFER_VK;
	EP.RND.SUBMIT_INFO_VK.signalSemaphoreCount = 1;
	EP.RND.SUBMIT_INFO_VK.pSignalSemaphores = &EP.RND.SEMAPHORE_IMAGE_AVAILABLE_VK;

	EP.RND.GRAPHICS_PIPELINE_CURRENT_VK = EP.RND.GRAPHICS_PIPELINE_VK;


	return true;
}

VkInstance createVkInstance(SDL_Window* window)
{
	// Query for required extensions using SDL_Vulkan_GetInstanceExtensions
	uint32_t extensionCount = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
		// Error handling
		return VK_NULL_HANDLE;
	}

	std::vector<const char*> extensionNames(extensionCount);
	if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data())) {
		// Error handling
		return VK_NULL_HANDLE;
	}

	// Create VkInstance
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extensionNames.data();

	VkInstance instance = VK_NULL_HANDLE;
	EP.RND.RESULT_VK = vkCreateInstance(&createInfo, nullptr, &instance);
	if (EP.RND.RESULT_VK != VK_SUCCESS) {
		// Error handling
		return VK_NULL_HANDLE;
	}

	return instance;
}

static int processPhysics(void* ptr)
{
	while (true)
	{
		if (EP.EXECUTE.isPhysicsThreadActive)
		{
			if (getPhysicsReady(PHYSICS_TYPE_PLAYER_MOVEMENT))
			{
				CLIENT.handleEvent(e,MEM.MAP.CURRENT_MAP->getMapSize());

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

			handleCollision();
		}
		SDL_Delay(SDL_GLOBAL_DELAY);
	}
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
	CLIENT.resetData();

	for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed())
		{
			Player[i].resetData();
		}
	}
}

void matchResultScreen(bool clientWin)
{
	cout << endl << "TRIGGERED";

	EP.EXECUTE.exitCurrentLoop = false;
	EP.GSYS.matchResultTimer.start();

	while (EP.GSYS.matchResultTimer.getTicks() < 3000 && !EP.EXECUTE.exitCurrentLoop)
	{
		while (SDL_PollEvent(&e))
		{
			SDL_RenderClear(gWindow.getRenderer());
			SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

			background_texture.render(gWindow.getRenderer(), 0, 0, &EP.GSYS.RESOLUTION_CLIP, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

			if (clientWin)
			{
				MEM.TEXTR.MATCH_RESULT_WON.render(gWindow.getRenderer(), gWindow.getWidth() / 2 - MEM.TEXTR.MATCH_RESULT_WON.getWidth()/2, gWindow.getHeight() / 2 - MEM.TEXTR.MATCH_RESULT_WON.getHeight()/2, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
			}
			else
			{
				MEM.TEXTR.MATCH_RESULT_LOST.render(gWindow.getRenderer(), gWindow.getWidth() / 2 - MEM.TEXTR.MATCH_RESULT_LOST.getWidth()/2, gWindow.getHeight() / 2 - MEM.TEXTR.MATCH_RESULT_LOST.getHeight()/2, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
			}

			gWindow.handleEvent(e);
		}

		SDL_Delay(1);
		gWindow.render();
	}
	tryLoopExit();
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
	SDL_Rect collisionRect;

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

	//Collisions between player and map objects

	for (unsigned int i = 0; i < MEM.MAP.CURRENT_MAP->getMaxObjects(); i++)
	{
		if (MEM.MAP.CURRENT_MAP->getObjectSlotUsed(i) && MEM.MAP.CURRENT_MAP->getTextureCollisionBool(i))
		{
			if (checkCollision(CLIENT.getmCollider(), MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i)))
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


	//CHECKING FOR COLLISIONS BETWEEN PROJECTILES AND MAP OBJECTS

	for (int i = 0; i < MEM.MAP.CURRENT_MAP->getMaxObjects(); i++)
	{
		if (MEM.MAP.CURRENT_MAP->getObjectSlotUsed(i) && MEM.MAP.CURRENT_MAP->getTextureCollisionBool(i))
		{
			for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
			{
				if (!CLIENT.gProjectile[j].getSlotFree())
				{
					if (checkCollision(CLIENT.gProjectile[j].getCollisionRect(), MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i)))
					{
						CLIENT.gProjectile[j].setSlotFree(true);
						ANIM_FIREBALL.setCurrentTickClient(j, CLIENT_UNIQUE_ID, 0);
						ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim(MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i).x, MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i).y, true, false);
					}
				}
			}

			//CHECK FOR PROJECTILES OF OTHER PLAYERS

			for (unsigned int j = 0; j < MAX_PLAYER_ENTITY; j++)
			{
				if (Player[j].getIfSlotUsed() && !Player[j].getPlayerDead())
				{
					for (unsigned int k = 0; k < MAX_PLAYER_BULLET_COUNT; k++)
					{
						if (!Player[j].gProjectile[k].getSlotFree())
						{
							if (checkCollision(Player[j].gProjectile[k].getCollisionRect(), MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i)))
							{
								Player[j].gProjectile[k].setSlotFree(true);
								ANIM_FIREBALL.setCurrentTickClient(j, k, 0);
								ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim(MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i).x, MEM.MAP.CURRENT_MAP->getTextureCollisionRect(i).y, true, false);
							}
						}
					}
				}
			}
		}
	}

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
						if (!Player[i].gProjectile[j].getSlotFree() && !ANIM_FIREBALL.getIsInverseSeq(i, j))
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
						ANIM_FIREBALL.setCurrentTickClient(i, j, 0);
						ANIM_CONTACT_REDEXPLOSION.addNewStaticAnim((Player[i].gProjectile[j].getPosX() + CLIENT.getCollisionRect().x) / 2, (Player[i].gProjectile[j].getPosY() + CLIENT.getCollisionRect().y) / 2, true, false);
						cout << endl << CLIENT.getHealth();
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

int clientSendData(const string& input)
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
	SDL_RenderClear(gWindow.getRenderer());
	SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

	flipType = CLIENT.getCharDir() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	CLIENT.setFlipTypeString(sFlipType = flipType == SDL_FLIP_HORIZONTAL ? "horizontal" : "none");

	//RENDER GROUND

   	MEM.MAP.CURRENT_MAP->renderMap(gWindow.getRenderer(), CLIENT.getPlayerPoint());

	EP.TEMP.CAMERA_X = CLIENT.getPosX() - MEM.MAP.CURRENT_MAP->getCamera().x;
	EP.TEMP.CAMERA_Y = CLIENT.getPosY() - MEM.MAP.CURRENT_MAP->getCamera().y;

	//RENDER CLIENT PLAYER

	if (ANIM_RUNNING_ATTACK.getInUse())
	{
		ANIM_RUNNING_ATTACK.renderTexture(gWindow.getRenderer(), EP.TEMP.CAMERA_X, EP.TEMP.CAMERA_Y, 0, 0, CLIENT_UNIQUE_ID, false, flipType, 500, 500, EP.EXECUTE.renderCollisionBox, CLIENT.getCollisionRect().w, CLIENT.getCollisionRect().h, EP.TEMP.CAMERA_X + CLIENT.getCollisionRect().w / 2, EP.TEMP.CAMERA_Y + CLIENT.getCollisionRect().h / 2);
		CLIENT.setAnimType("runAttack");
	}
	else if (CLIENT.getIfMoving())
	{
		ANIM_WALKING.renderTexture(gWindow.getRenderer(), EP.TEMP.CAMERA_X, EP.TEMP.CAMERA_Y, 0, 0, CLIENT_UNIQUE_ID, false, flipType, 500, 500, EP.EXECUTE.renderCollisionBox, CLIENT.getCollisionRect().w, CLIENT.getCollisionRect().h, EP.TEMP.CAMERA_X + CLIENT.getCollisionRect().w/2, EP.TEMP.CAMERA_Y + CLIENT.getCollisionRect().h/2);
		CLIENT.setAnimType("walking");
	}
	else
	{
		ANIM_IDLE.renderTexture(gWindow.getRenderer(), EP.TEMP.CAMERA_X, EP.TEMP.CAMERA_Y, 0, 0, CLIENT_UNIQUE_ID, false, flipType, 500, 500, EP.EXECUTE.renderCollisionBox, CLIENT.getCollisionRect().w, CLIENT.getCollisionRect().h, EP.TEMP.CAMERA_X + CLIENT.getCollisionRect().w / 2, EP.TEMP.CAMERA_Y + CLIENT.getCollisionRect().h / 2);
		CLIENT.setAnimType("idle");
	}

	//RENDER PLAYER PROJECTILES

	if (CLIENT.getProjectileActive())
	{
		for (unsigned int i = 0; i < MAX_PLAYER_BULLET_COUNT; i++)
		{
			if (!CLIENT.gProjectile[i].getSlotFree())
			{
				ANIM_FIREBALL.renderTexture(gWindow.getRenderer(), CLIENT.gProjectile[i].getPosX() - MEM.MAP.CURRENT_MAP->getCamera().x, CLIENT.gProjectile[i].getPosY() - MEM.MAP.CURRENT_MAP->getCamera().y, CLIENT.gProjectile[i].getAngle(), i, CLIENT_UNIQUE_ID, false, SDL_FLIP_NONE, EP.ANIM.FIREBALL_RENDERSPEED, 1, EP.EXECUTE.renderCollisionBox, CLIENT.gProjectile[i].getCollisionRect().w, CLIENT.gProjectile[i].getCollisionRect().h, CLIENT.gProjectile[i].getCollisionRect().x, CLIENT.gProjectile[i].getCollisionRect().y);
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
					ANIM_FIREBALL.renderTexture(gWindow.getRenderer(), Player[i].gProjectile[j].getPosX() - MEM.MAP.CURRENT_MAP->getCamera().x, Player[i].gProjectile[j].getPosY() - MEM.MAP.CURRENT_MAP->getCamera().y, Player[i].gProjectile[j].getAngle(), i, j, false, SDL_FLIP_NONE, EP.ANIM.FIREBALL_RENDERSPEED, 1, EP.EXECUTE.renderCollisionBox, CLIENT.gProjectile[i].getCollisionRect().w, CLIENT.gProjectile[i].getCollisionRect().h, CLIENT.gProjectile[i].getCollisionRect().x, CLIENT.gProjectile[i].getCollisionRect().y);
				}
			}
		}
	}

	//RENDER WALKING STATE

	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
		{
			EP.TEMP.CAMERA_X = Player[i].getPosX() - MEM.MAP.CURRENT_MAP->getCamera().x;
			EP.TEMP.CAMERA_Y = Player[i].getPosY() - MEM.MAP.CURRENT_MAP->getCamera().y;


			if (Player[i].getAnimType() == "idle")
			{
				ANIM_IDLE.renderTexture(gWindow.getRenderer(), EP.TEMP.CAMERA_X, EP.TEMP.CAMERA_Y, 0, i, 0, false, Player[i].getFlipType(), 500, 500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_IDLE.updateAnim(i, 0);
			}
			else if (Player[i].getAnimType() == "runAttack")
			{
				ANIM_RUNNING_ATTACK.renderTexture(gWindow.getRenderer(), EP.TEMP.CAMERA_X, EP.TEMP.CAMERA_Y, 0, i, 0, false, Player[i].getFlipType(), 500, 500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_RUNNING_ATTACK.updateAnim(i, 0);
			}
			else if (Player[i].getAnimType() == "walking")
			{
				ANIM_WALKING.renderTexture(gWindow.getRenderer(), EP.TEMP.CAMERA_X, EP.TEMP.CAMERA_Y, 0, i, 0, false, Player[i].getFlipType(), 500, 500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_WALKING.updateAnim(i, 0);
			}

			nickname_text_texture.loadFromRenderedText(Player[i].getNickname(), gColor, gWindow.getRenderer(), gNorthFont);
			nickname_text_texture.render(gWindow.getRenderer(), (EP.TEMP.CAMERA_X + 35) - (strlen(Player[i].getNickname().c_str())), EP.TEMP.CAMERA_Y, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
		}
	}


	//RENDER EXPOSION COLLISIONS

	ANIM_CONTACT_REDEXPLOSION.renderStaticAnim(gWindow.getRenderer(), MEM.MAP.CURRENT_MAP->getCamera().x, MEM.MAP.CURRENT_MAP->getCamera().y);

	//RENDER CROSSHAIR

	crosshair_texture.render(gWindow.getRenderer(), mouseX - crosshair_texture.getWidth() / 2, mouseY - (crosshair_texture.getHeight() / 2) + 40, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

	//RENDER FPS COUNTER

	MEM.TEXTR.fpsText.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

	//Render to window

	if (EP.EXECUTE.INSERT_OBJECT)
	{
		MEM.TEXTR.NEW_OBJECT.renderSimple(gWindow.getRenderer(), EP.TEMP.MOUSE_X, EP.TEMP.MOUSE_Y);
	}


	gWindow.render();

}

void createBigTexture(LTexture &sTexture, LTexture &tTexture,int mapSizeX, int mapSizeY)
{
	SDL_Rect textureClip = { 0,0,sTexture.getWidth(),sTexture.getHeight() };
	SDL_Point tSize;

	tTexture.loadTargetTexture(gWindow.getRenderer(), mapSizeX, mapSizeY);

	for (int i = 0; i <= (int)mapSizeX /sTexture.getWidth(); i++)
	{
		for (int j = 0; j <= (int)mapSizeY / sTexture.getHeight(); j++)
		{
			sTexture.render(gWindow.getRenderer(), i * sTexture.getWidth(), j * sTexture.getHeight(), &textureClip, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
		}
	}
	SDL_SetRenderTarget(gWindow.getRenderer(), NULL);

	SDL_QueryTexture(tTexture.getTexture(), NULL, NULL, &tSize.x, &tSize.y);

	tTexture.setTextureSize(tSize);
}

int sendPacket(void* ptr)
{


	return 0;
}

bool init()
{
	bool success = true;

	int temp;
	string stemp;

	//fstream gameSettings("data/gameSettings.txt", ios::in);
	////EP.FSTR.gameLog.open("data/gameLog.txt", ios::out | ios::app);
	//
	//gameSettings >> stemp >> temp;
	//gWindow.setWidth(temp);
	//gameSettings >> stemp >> temp;
	//gWindow.setHeight(temp);
	//gameSettings >> stemp >> temp;
	//gWindow.setFullscreen(temp);
	//
	//gameSettings.close();

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

			user_textbox_button.setPosition(565, 300, 200, 32);
			pass_textbox_button.setPosition(565, 370, 200, 32);
			MEM.BTT.TWO_BUTTON.setPosition(350, 300, 290, 80);
			MEM.BTT.FOUR_BUTTON.setPosition(650, 300, 290, 80);

			register_button.setPosition(430, 430, 100, 30);
			login_button.setPosition(760, 435, 100, 30);

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

	iResult = getaddrinfo("192.168.100.3", DEFAULT_PORT, &hints, &result);
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
	if (!MEM.TEXTR.BRICK_FLOOR.loadFromFile("img/surfaces/map_test.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.MATCHING_IN_PROGRESS.loadFromFile("img/matchingScreen/matching.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.MATCH_RESULT_WON.loadFromFile("img/matchingScreen/match_won.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.MATCH_RESULT_LOST.loadFromFile("img/matchingScreen/match_lost.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!MEM.TEXTR.GRASS_BG.loadFromFile("img/surfaces/grass_bg4.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}

	//LOAD BUTTONS

	MEM.BTT.LOGIN.init("img/loginButton.png", gWindow.getRenderer(), gWindow.getWidth() / 1.95f, gWindow.getHeight() / 1.80f);
	MEM.BTT.REGISTER.init("img/registerButton.png", gWindow.getRenderer(), gWindow.getWidth() / 2.45f, gWindow.getHeight() / 1.80f);
	MEM.BTT.MATCHING_2PLAYER.init("img/matchingScreen/matching_2player.png", gWindow.getRenderer(), gWindow.getWidth() * 0.34f, gWindow.getHeight() * 0.50f);
	MEM.BTT.MATCHING_4PLAYER.init("img/matchingScreen/matching_4player.png", gWindow.getRenderer(), gWindow.getWidth() * 0.50f, gWindow.getHeight() * 0.50f);

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

	//MAP INIT

	//GRASS WORLD

	MEM.MAP.GRASS_WORLD.initMap(gWindow.getRenderer(), "img/MAP_TEXTURES/GRASS_WORLD/texture_000.png", 50, "data/grass_world_data" ,4000, 4000, gWindow.getWidth(), gWindow.getHeight(), MEM.TEXTR.PAWN_COLLISION_REFERENCE.getWidth(), MEM.TEXTR.PAWN_COLLISION_REFERENCE.getHeight());

	createBigTexture(MEM.TEXTR.GRASS_BG, *MEM.MAP.GRASS_WORLD.getBackgroundTexture(), MEM.MAP.GRASS_WORLD.getMapSize().x, MEM.MAP.GRASS_WORLD.getMapSize().y);

	MEM.MAP.GRASS_WORLD.setMapBackground(*MEM.MAP.GRASS_WORLD.getBackgroundTexture());

	//Init stuff

	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		Player[i].setMCWH(49, 65);
		Player[i].setCollisionRectWH(49, 65);
		Player[i].setCollisionOffset(25, 20);

		for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
		{
			Player[i].gProjectile[j].setMCWH(27, 27);
			Player[i].gProjectile[j].setCollisionRectWH(27, 27);
			Player[i].gProjectile[j].setProjSpeed(EP.GSYS.DEFAULT_PROJ_SPEED);
		}
	}
	for (unsigned int i = 0; i < 30; i++)
	{
		CLIENT.gProjectile[i].setMCWH(27, 27);
		CLIENT.gProjectile[i].setCollisionRectWH(27, 27);
		CLIENT.gProjectile[i].setProjSpeed(EP.GSYS.DEFAULT_PROJ_SPEED);
	}

	CLIENT.setCollisionOffset(25, 20);
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

		if (!MEM.TEXTR.fpsText.loadFromRenderedText("FPS", gColor, gWindow.getRenderer(), gNorthFont))
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

string getData(int len, char* data)
{
	string sdata(data);
	sdata = sdata.substr(0, len);
	return sdata;
}

int recivePacket(void* ptr)
{

	return 0;
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

void testEnviroment()
{
	while (EP.EXECUTE.exitCurrentLoop == false)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				EP.EXECUTE.exitCurrentLoop = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_SPACE)
				{
					VK.switchPipeline();
				}	
			}
		}
		gWindow.handleEvent(e);

		vkRender();

	}
}

int main(int argc, char* args[])
{
	

	if (!init())
	{
		printf("Failed to initialize SDL!\n");
	}
	//else if (!initVulkan())
	//{
	//	printf("Failed to initialize VULKAN!\n");
	//}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			srand(time(NULL));

			//THREAD.PHYSICS = SDL_CreateThread(processPhysics, "processPhysics", (void*)NULL);
			testEnviroment();

		}
	}
	close();

	return 0;
}
//end