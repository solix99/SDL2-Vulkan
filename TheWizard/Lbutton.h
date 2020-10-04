#pragma once

#include <SDL.h>
#include "LTexture.h"


class LButton
{
public:
	//Initializes internal variables
	LButton();

	//Sets top left position
	void setPosition(int x, int y,int w,int h);

	//Handles mouse event
	bool handleClick(SDL_Event e);

	//Shows button sprite
	void render(LTexture object, SDL_Renderer* gRenderer);

private:
	//Top left position
	SDL_Rect mPosition;

};
