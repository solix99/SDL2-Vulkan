#pragma once

#include <SDL.h>
#include "LTexture.h"
#include <string>

class LButton
{
public:
	//Initializes internal variables
	LButton();

	//Sets top left position
	void setPosition(int x, int y,int w,int h);

	void init(string path,SDL_Renderer * gRenderer,int posX, int posY);

	//Handles mouse event
	bool handleClick(SDL_Event e);

	//Shows button sprite
	void render(SDL_Renderer* gRenderer);

private:
	//Top left position

	SDL_Rect mPosition;
	LTexture TEXTURE_;

};
