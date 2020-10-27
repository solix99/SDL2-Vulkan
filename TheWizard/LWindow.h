#pragma once

#include <SDL.h>
#include <iostream>
#include <vulkan/vulkan.h>

class LWindow
{
public:
	//Intializes internals
	LWindow(int, int);
	~LWindow();

	//Creates window
	bool init();

	//Handles window events
	void handleEvent(SDL_Event& e);

	//Focuses on window
	void focus();

	//Shows windows contents
	void render();

	//Deallocates internals
	void free();

	//Window dimensions
	int getWidth();
	int getHeight();

	void setWidth(int);
	void setHeight(int);

	//Window focus
	bool hasMouseFocus();
	bool hasKeyboardFocus();
	bool isMinimized();
	bool isShown();

	SDL_Renderer* getRenderer();
	
	void setFullscreen(bool);

	SDL_Window* getWindow();

	VkInstance &getVulkanInstance();

private:
	//Window data
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	int mWindowID;

	//Window dimensions
	int mWidth;
	int mHeight;

	//Window focus
	bool mMouseFocus;
	bool mKeyboardFocus;
	bool mFullScreen;
	bool mMinimized;
	bool mShown;
	
	VkInstance vInstance;
};