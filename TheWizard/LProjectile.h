#pragma once

#include <iostream>
#include <SDL.h>
#include <cmath>
#include "LAnim.h"

class LProjectile
{
public:
	LProjectile();

	int DISTANCE;

	float getPosX();
	float getPosY();
	SDL_Rect getmCollider();
	SDL_Rect getBulletPosRect(int);
	void setPosX(float);
	void setPosY(float);
	void setMCWH(int, int);
	bool checkCollision(SDL_Rect a, SDL_Rect b);
	bool getSlotFree();
	void setSlotFree(bool);
	void setAngle(float);
	bool checkStatus();
	float getAngle();
	int getDestX();
	int getDestY();
	float getVelX();
	float getVelY();
	int getDMG();
	void setDestX(int);
	void setDestY(int);
	void setVelX(float);
	void setVelY(float);
	void setDMG(int);
	void setCollisionRectWH(int W, int H);
	void setCollisionOffset(float x, float y);
	SDL_Rect getCollisionRect();
	float getXCollisionOffset();
	float getYCollisionOffset();
	float getProjSpeed();
	void setProjSpeed(float pSpeed);

protected:

private:

	float angle;
	int DEFAULT_VEL = 5;
	float mPosX, mPosY;
	int p_collisionAmmount = 10;
	int p_DMG;
	float xCollisionOffset, yCollisionOffset;

	int mDestX, mDestY;
	float mVelX, mVelY;
	float PROJ_SPEED_;

	SDL_Rect p_collisionRect;
	SDL_Rect mCollider;
	bool slotFree;

};
