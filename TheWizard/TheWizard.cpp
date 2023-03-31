// TheWizard.cpp : This file contains the 'main' function. Program execution begins and ends there.

#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION

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
#include "Vars.h"
#include "vk_mem_alloc.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


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
	struct CAMERA
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 mesh_matrix;
		glm::vec3 camPos{0.f, 0.f, -10.f};
		glm::vec3 camFront = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

	}CAM;

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
		VkPipelineVertexInputStateCreateInfo VERTEX_INPUT_INFO = {};
		struct MeshPushConstants {
			glm::vec4 data;
			glm::mat4 render_matrix;
		}MPC;

		int FRAME_NUMBER = 0;
		int FRAME_NUMBER_DEPTH = 0;

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
void endSingleTimeCommands(VkCommandBuffer commandBuffer);
void handleCamera();



void handleCamera()
{
	// Calculate view matrix
	EP.CAM.view = glm::lookAt(EP.CAM.camPos, EP.CAM.camPos + EP.CAM.camFront, EP.CAM.camUp);

	// Calculate projection matrix with depth information
	float nearPlane = 0.1f;
	float farPlane = 200.0f;
	float aspectRatio = 16.0f / 9.0f;
	float fov = 70.0f;
	float tanHalfFov = tan(glm::radians(fov / 2.0f));
	glm::mat4 projection = glm::mat4(0.0f);
	projection[0][0] = 1.0f / (aspectRatio * tanHalfFov);
	projection[1][1] = 1.0f / tanHalfFov;
	projection[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
	projection[2][3] = -1.0f;
	projection[3][2] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);

	// Modify projection matrix to invert y-axis to match OpenGL coordinate system
	projection[1][1] *= -1;

	// Calculate model matrix
	EP.CAM.model = glm::rotate(glm::mat4{ 1.f}, glm::radians(EP.RND.FRAME_NUMBER * 0.4f), glm::vec3(0, 1, 0));

	// Combine view, projection, and model matrices
	EP.CAM.mesh_matrix = projection * EP.CAM.view * EP.CAM.model;


	EP.RND.FRAME_NUMBER > 120 ? EP.RND.FRAME_NUMBER = 0 : EP.RND.FRAME_NUMBER++;

}

auto startTime = std::chrono::high_resolution_clock::now();

/*

void vkRender()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
	float frequency = elapsedTime / 8.33f; // 8.33 milliseconds in 1/120th second

	float flash = abs(sinf(frequency * 2.0f * glm::pi<float>())) / 2.0f + 0.5f; // Scale the output sin wave to range [0, 1]

	VkClearValue clearValue = {};
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	VkDeviceSize offset = 0;

	vkAcquireNextImageKHR(VK.getLogicalDevice(), VK.getSwapchain(), UINT64_MAX, VK.getSemaphoreWait(), VK_NULL_HANDLE, VK.getImageIndex());

	VK.getRenderPassBeginInfo()->framebuffer = VK.getSwapchainFramebuffer(*VK.getImageIndex());

	// begin recording commands
	vkBeginCommandBuffer(VK.getCommandBuffer(), VK.getCommandBufferBeginInfo());

	cout << endl << flash;

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = VK.getRenderPassBeginInfoEx();

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(VK.getCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(VK.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, VK.getCurrentPipeline());

	vkCmdBindVertexBuffers(VK.getCommandBuffer(), 0, 1, VK.MESH.getVertexBuffer(), &offset);

	struct MeshPushConstants {
		glm::vec4 data;
		glm::mat4 render_matrix;
	};

	MeshPushConstants constants;
	constants.render_matrix = EP.CAM.mesh_matrix;

	vkCmdPushConstants(VK.getCommandBuffer(), VK.getPipelineLayout(VK.MESH), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

	vkCmdDraw(VK.getCommandBuffer(), VK.MESH.getVerticesSize(), 1, 0, 0);

	vkCmdEndRenderPass(VK.getCommandBuffer());

	// end recording commands
	vkEndCommandBuffer(VK.getCommandBuffer());

	vkQueueSubmit(VK.getGraphicsQueue(), 1, VK.getSubmitInfo(), VK_NULL_HANDLE);

	//vkWaitForFences(VK.getLogicalDevice(), 1, VK.getFenceRenderingFinished(), VK_TRUE, UINT64_MAX);

	//vkResetFences(VK.getLogicalDevice(), 1, VK.getFenceRenderingFinished());

	vkQueuePresentKHR(VK.getGraphicsQueue(), VK.getPresentInfo());
}

*/


void vkRender()
{
	VkDeviceSize offset = 0;

	//vkCmdClearDepthStencilImage(VK.getCommandBuffer(),VK.getSwapchain(),);

	vkAcquireNextImageKHR(VK.getLogicalDevice(), VK.getSwapchain(), UINT64_MAX, VK.getSemaphoreWait(), VK_NULL_HANDLE, VK.getImageIndex());

	VK.getRenderPassBeginInfo()->framebuffer = VK.getSwapchainFramebuffer(*VK.getImageIndex());


	vkBeginCommandBuffer(VK.getCommandBuffer(), VK.getCommandBufferBeginInfo());



	VkClearValue clearValuesx[2];
	clearValuesx[0] = { 0.0f, 0.0f, 0.0f, 1.0f }; // or whatever your first clear value is
	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = VK.getRenderPassBeginInfoEx();

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValuesx[0], depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(VK.getCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	// begin recording commands


	//vkCmdBeginRenderPass(VK.getCommandBuffer(), VK.getRenderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(VK.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, VK.getCurrentPipeline());


	vkCmdBindVertexBuffers(VK.getCommandBuffer(), 0, 1, VK.MESH.getVertexBuffer(), &offset);

	struct MeshPushConstants {
		glm::vec4 data;
		glm::mat4 render_matrix;
	};

	MeshPushConstants constants;
	constants.render_matrix = EP.CAM.mesh_matrix;

	vkCmdPushConstants(VK.getCommandBuffer(), VK.getPipelineLayout(VK.MESH), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

	vkCmdDraw(VK.getCommandBuffer(), VK.MESH.getVerticesSize(), 1, 0, 0);

	vkCmdEndRenderPass(VK.getCommandBuffer());

	vkEndCommandBuffer(VK.getCommandBuffer());

	vkQueueSubmit(VK.getGraphicsQueue(), 1, VK.getSubmitInfo(), VK_NULL_HANDLE);

	vkQueuePresentKHR(VK.getGraphicsQueue(), VK.getPresentInfo());
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

void endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(VK.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(VK.getGraphicsQueue());

	vkFreeCommandBuffers(VK.getLogicalDevice(), VK.getCommandPool(), 1, &commandBuffer);
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

	//VK.initPipeline("PIPE1","shaders/colored_triangle_vertex.spv","shaders/colored_triangle_frag.spv");
	VK.initPipeline("PIPE2", "shaders/tri_mesh.spv", "shaders/colored_triangle_frag.spv",VK.MESH);

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
				if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					
				}	
				else if (e.key.keysym.sym == SDLK_LCTRL)
				{
					EP.CAM.camPos.y += 0.1f;
				}
				else if (e.key.keysym.sym == SDLK_SPACE)
				{
					EP.CAM.camPos.y -= 0.1f;
				}
				else if (e.key.keysym.sym == SDLK_a)
				{
					EP.CAM.camPos.x += 0.1f;
				}
				else if (e.key.keysym.sym == SDLK_d)
				{
					EP.CAM.camPos.x -= 0.1f;
				}
				else if (e.key.keysym.sym == SDLK_w)
				{
					EP.CAM.camPos.z += 0.1f;
				}
				else if (e.key.keysym.sym == SDLK_s)
				{
					EP.CAM.camPos.z -= 0.1f;
				}
				else if (e.key.keysym.sym == SDLK_r)
				{
					EP.CAM.camUp.y += 1.0f; // Move camera up by 1 unit
				}
				else if (e.key.keysym.sym == SDLK_f)
				{
					EP.CAM.camFront.z *= -1.0f; // Invert z-component of camFront to face the other way
				}
			}
		}
		gWindow.handleEvent(e);

		handleCamera();

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