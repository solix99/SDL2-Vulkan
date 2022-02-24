#pragma once
#include <SDL.h>
#include "Ltexture.h"
#include "LProjectile.h"
#include "LAnim.h"

class LPawn
{
public:
	LPawn();

	void setPosX(int);
	void setPosY(int);
	void setMCWH(int, int);
	void handleEvent(SDL_Event& e, SDL_Point mapSize);
	void setTexture(LTexture& gTexture);
	void setCharDIR(bool);
	LTexture getTexture();

	bool checkCollision(SDL_Rect a, SDL_Rect b);

	SDL_Rect getmCollider();
	SDL_Rect getBulletPosRect(int);

	void move();
	int getPosX();
	int getPosY();
	int getVelX();
	int getVelY();
	int getCharDir();
	string getNickname();
	string getPlayerID();
	bool getIfSlotUsed();
	bool getIfMoving();
	bool getProjectileActive();
	void setIfSlotUsed(bool);
	void spawnProjectile(int x, int y, int ang, int dx, int dy, float projSpeed);
	void setProjectileActive(bool);
	void setPlayerID(string);
	void setNickname(string);
	string getFlipTypeString();	
	string getID();
	SDL_RendererFlip getFlipType();
	string getAnimType();
	int getProjectilesActive();
	int getHealth();
	void setHealth(int);
	void setCollisionRectWH(int W, int H);
	SDL_Rect getCollisionRect();
	void damageTarget(int);
	void setFlipTypeString(string);
	void setFlipType(SDL_RendererFlip);
	void setAnimType(string);
	void setCollisionOffset(int x, int y);
	void setPlayerDead(bool b);
	bool getPlayerDead();
	void resetData();

	SDL_Point getPlayerPoint();

	LProjectile gProjectile[30];

	SDL_RendererFlip flipType;

private:

	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	LTexture mPawnTexture;

	bool charDir;
	bool moving;
	bool projectileActive;
	bool slotUsed;
	bool isDead;

	int DEFAULT_VEL = 5;
	int p_collisionAmmount = 10;
	SDL_Rect p_collisionRect;
	SDL_Rect mCollider;
	SDL_Point playerPoint;

	int mPosX, mPosY;
	
	float mVelX, mVelY;
	
	int TEXTURE_WIDTH, TEXTURE_HEIGHT;
	int xCollisionOffset;
	int yCollisionOffset;
	int p_health;
	int pDefaultHealth;

	string flipTypeString, animType;
	string playerID;
	string nickname;

};
