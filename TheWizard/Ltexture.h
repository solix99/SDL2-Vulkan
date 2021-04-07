#pragma once

#include <SDL.h>
#include <string>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdlib.h>

using namespace std;


class LTexture
{
public:
	LTexture();

	~LTexture();

	bool loadFromFile(std::string path, SDL_Renderer* gRenderer);

	bool loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* gRenderer, TTF_Font* gFont);

	bool loadTargetTexture(SDL_Renderer* gRenderer,int sizeX,int sizeY,int textureSizeW,int textureSizeH);

	void free();

	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	void setBlendMode(SDL_BlendMode blending);

	void setAlpha(Uint8 alpha);

	void render(SDL_Renderer* gRenderer, int x, int y, SDL_Rect* clip , double angle, SDL_Point * center , SDL_RendererFlip flip,bool renderCollisionBox, int colW, int colH,int colX, int colY);

	SDL_Texture *getTexture();

	int getWidth();
	int getHeight();

private:
	SDL_Texture* mTexture;

	int mWidth;
	int mHeight;

	SDL_Rect collisionBox;

};

