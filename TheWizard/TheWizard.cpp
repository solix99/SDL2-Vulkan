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
		LTimer matchResultTimer;
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
		bool MATCH_RESULT_SCREEN = false;

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
		LTexture MATCH_RESULT_WON;
		LTexture MATCH_RESULT_LOST;

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
void matchResultScreen();

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


		//	cout << endl << CLIENT.getHealth();

			//Handle collision

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
	EP.GSYS.matchResultTimer.start();

	while (EP.GSYS.matchResultTimer.getTicks() < 3000 && !EP.EXECUTE.exitCurrentLoop)
	{
		while (SDL_PollEvent(&e))
		{
			SDL_RenderClear(gWindow.getRenderer());
			SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

			background_texture.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

			if (clientWin)
			{
				MEM.TEXTR.MATCH_RESULT_WON.render(gWindow.getRenderer(), DEFAULT_RESOLUTION_WIDTH / 2 - MEM.TEXTR.MATCH_RESULT_WON.getWidth(), DEFAULT_RESOLUTION_HEIGHT / 2 - MEM.TEXTR.MATCH_RESULT_WON.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
			}
			else
			{
				MEM.TEXTR.MATCH_RESULT_LOST.render(gWindow.getRenderer(), DEFAULT_RESOLUTION_WIDTH / 2 - MEM.TEXTR.MATCH_RESULT_LOST.getWidth(), DEFAULT_RESOLUTION_HEIGHT / 2 - MEM.TEXTR.MATCH_RESULT_LOST.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
			}

			gWindow.render();
			gWindow.handleEvent(e);

			SDL_Delay(1);
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
	flipType = CLIENT.getCharDir() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	CLIENT.setFlipTypeString(sFlipType = flipType == SDL_FLIP_HORIZONTAL ? "horizontal" : "none");

	SDL_RenderClear(gWindow.getRenderer());
	SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

	//RENDER GROUND

	texture_brickFloor.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth(), 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
	texture_brickFloor.render(gWindow.getRenderer(), 0, texture_brickFloor.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth(), texture_brickFloor.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth() * 2, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
	texture_brickFloor.render(gWindow.getRenderer(), texture_brickFloor.getWidth() * 2, texture_brickFloor.getHeight(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

	//RENDER CLIENT PLAYER

	if (ANIM_RUNNING_ATTACK.getInUse())
	{
		ANIM_RUNNING_ATTACK.renderTexture(gWindow.getRenderer(), CLIENT.getPosX(), CLIENT.getPosY(), 0, 0, CLIENT_UNIQUE_ID, false, flipType, 500, 50, EP.EXECUTE.renderCollisionBox, CLIENT.getCollisionRect().w, CLIENT.getCollisionRect().h, CLIENT.getCollisionRect().x, CLIENT.getCollisionRect().y);
		CLIENT.setAnimType("runAttack");
	}
	else if (CLIENT.getIfMoving())
	{
		ANIM_WALKING.renderTexture(gWindow.getRenderer(), CLIENT.getPosX(), CLIENT.getPosY(), 0, 0, CLIENT_UNIQUE_ID, false, flipType, 500, 500, EP.EXECUTE.renderCollisionBox, CLIENT.getCollisionRect().w, CLIENT.getCollisionRect().h, CLIENT.getCollisionRect().x, CLIENT.getCollisionRect().y);
		CLIENT.setAnimType("walking");
	}
	else
	{
		ANIM_IDLE.renderTexture(gWindow.getRenderer(), CLIENT.getPosX(), CLIENT.getPosY(), 0, 0, CLIENT_UNIQUE_ID, false, flipType, 500, 500, EP.EXECUTE.renderCollisionBox, CLIENT.getCollisionRect().w, CLIENT.getCollisionRect().h, CLIENT.getCollisionRect().x, CLIENT.getCollisionRect().y);
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
		//cout << endl << Player[0].getPlayerDead();

		if (Player[i].getIfSlotUsed() && !Player[i].getPlayerDead())
		{
			if (Player[i].getAnimType() == "idle")
			{
				ANIM_IDLE.renderTexture(gWindow.getRenderer(), Player[i].getPosX(), Player[i].getPosY(), 0, i, 0, false, Player[i].getFlipType(), 500, 500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_IDLE.updateAnim(i, 0);
			}
			else if (Player[i].getAnimType() == "runAttack")
			{
				ANIM_RUNNING_ATTACK.renderTexture(gWindow.getRenderer(), Player[i].getPosX(), Player[i].getPosY(), 0, i, 0, false, Player[i].getFlipType(), 500, 500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_RUNNING_ATTACK.updateAnim(i, 0);
			}
			else if (Player[i].getAnimType() == "walking")
			{
				ANIM_WALKING.renderTexture(gWindow.getRenderer(), Player[i].getPosX(), Player[i].getPosY(), 0, i, 0, false, Player[i].getFlipType(), 500, 500, EP.EXECUTE.renderCollisionBox, Player[i].getCollisionRect().w, Player[i].getCollisionRect().h, Player[i].getCollisionRect().x, Player[i].getCollisionRect().y);
				ANIM_WALKING.updateAnim(i, 0);
			}

			nickname_text_texture.loadFromRenderedText(Player[i].getNickname(), gColor, gWindow.getRenderer(), gNorthFont);
			nickname_text_texture.render(gWindow.getRenderer(), (Player[i].getPosX() + 35) - (strlen(Player[i].getNickname().c_str())), Player[i].getPosY(), NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
		}
	}


	//RENDER EXPOSION COLLISIONS

	ANIM_CONTACT_REDEXPLOSION.renderStaticAnim(gWindow.getRenderer(), EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

	//RENDER CROSSHAIR

	crosshair_texture.render(gWindow.getRenderer(), mouseX - crosshair_texture.getWidth() / 2, mouseY - (crosshair_texture.getHeight() / 2) + 40, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

	//RENDER FPS COUNTER

	MEM.TEXTR.fpsText.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

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

	iResult = getaddrinfo("192.168.1.6", DEFAULT_PORT, &hints, &result);
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

		background_texture.render(gWindow.getRenderer(), 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

		user_text_texture.loadFromRenderedText(user_ss.c_str(), gColor, gWindow.getRenderer(), gNorthFont);
		pass_text_texture.loadFromRenderedText(pStar.c_str(), gColor, gWindow.getRenderer(), gNorthFont);

		loginPage_texture.render(gWindow.getRenderer(), gWindow.getWidth() * 0.5f - loginPage_texture.getWidth() / 2, gWindow.getHeight() * 0.5f - loginPage_texture.getHeight() / 2, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

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

		user_text_texture.render(gWindow.getRenderer(), gWindow.getWidth() * scaleX * 0.80f, gWindow.getHeight() * scaleX * 0.80f, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
		pass_text_texture.render(gWindow.getRenderer(), gWindow.getWidth() * scaleX * 0.80f, gWindow.getHeight() * scaleX * 0.916f, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

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
			green_textbox_texture.render(gWindow.getRenderer(), 555, 425, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

			info_text_texture.render(gWindow.getRenderer(), 575, 435, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

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
			red_textbox_texture.render(gWindow.getRenderer(), 555, 425, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);

			info_text_texture.render(gWindow.getRenderer(), 575, 435, NULL, NULL, NULL, SDL_FLIP_NONE, EP.EXECUTE.renderCollisionBox, 0, 0, 0, 0);
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

string getFinalData(const string& data)
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

		//	cout << endl << data;

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
				ID[0] = getFinalData(data);
				nickname[0] = getFinalData(data);

				if (ID[0] != gServer.getClientID())
				{
					for (int j = 0; j < MAX_PLAYER_ENTITY; j++)
					{
						if (!Player[j].getIfSlotUsed())
						{
							cout << endl << "NEW PLAYER ON ID:" << j;
							Player[j].setIfSlotUsed(true);
							Player[j].setPlayerID(ID[0]);
							Player[j].setNickname(nickname[0]);
							Player[j].setPlayerDead(false);
							break;
						}
					}
				}
				
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
								Player[j].setPlayerDead(false);
								//cout << endl << "NEW PLAYER ON ID:" << j << " MAX:" << mTypePC;

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
					EP.EXECUTE.MATCH_RESULT_SCREEN = true;
					EP.TEMP.MATCH_RESULT_WON = true;
					resetPlayerData();
					tryLoopExit();
				}
				else
				{
					EP.EXECUTE.MATCH_RESULT_SCREEN = true;
					EP.TEMP.MATCH_RESULT_WON = false;
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
			else if (e.type == SDL_MOUSEBUTTONDOWN)
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

		if (EP.EXECUTE.MATCH_RESULT_SCREEN)
		{
			matchResultScreen(EP.TEMP.MATCH_RESULT_WON);
			EP.EXECUTE.MATCH_RESULT_SCREEN = false;
		}

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
		}
	}
	close();

	return 0;
}

//end