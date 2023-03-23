#include "LWindow.h"


LWindow::LWindow(int w, int h)
	:mWidth(w), mHeight(h)
{
	//ctor
}

bool LWindow::init()
{
	//Create window
	if (mFullScreen)
	{
		mWindow = SDL_CreateWindow("The Wizard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mWidth, mHeight, SDL_WINDOW_VULKAN);
	}
	else
	{
		mWindow = SDL_CreateWindow("The Wizard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mWidth, mHeight, SDL_WINDOW_VULKAN);
	}

	
	if (mWindow != NULL)
	{
		mMouseFocus = true;
		mKeyboardFocus = true;

		//Create renderer for window
		mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
		if (mRenderer == NULL)
		{
			std::cout << std::endl << "Renderer could not be created! SDL Error:\n" << SDL_GetError();
			SDL_DestroyWindow(mWindow);
			mWindow = NULL;
		}
		else
		{
			//Initialize renderer color
			SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

			//Grab window identifier
			mWindowID = SDL_GetWindowID(mWindow);

			//Flag as opened
			mShown = true;

		}
	}
	else
	{
		std::cout << std::endl << "Window could not be created! SDL Error: \n" << SDL_GetError();
	}

	return mWindow != NULL && mRenderer != NULL;
}

SDL_Window* LWindow::getWindow()
{
	return mWindow;
}

void LWindow::handleEvent(SDL_Event& e)
{
	//If an event was detected for this window
	if (e.type == SDL_WINDOWEVENT && e.window.windowID == mWindowID)
	{
		//Caption update flag
		bool updateCaption = false;
		switch (e.window.event)
		{
			//Window appeared
		case SDL_WINDOWEVENT_SHOWN:
			mShown = true;
			break;

			//Window disappeared
		case SDL_WINDOWEVENT_HIDDEN:
			mShown = false;
			break;

			//Get new dimensions and repaint
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			mWidth = e.window.data1;
			mHeight = e.window.data2;
			SDL_RenderPresent(mRenderer);
			break;

			//Repaint on expose
		case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(mRenderer);
			break;

			//Mouse enter
		case SDL_WINDOWEVENT_ENTER:
			mMouseFocus = true;
			updateCaption = true;
			break;

			//Mouse exit
		case SDL_WINDOWEVENT_LEAVE:
			mMouseFocus = false;
			updateCaption = true;
			break;

			//Keyboard focus gained
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			mKeyboardFocus = true;
			updateCaption = true;
			break;

			//Keyboard focus lost
		case SDL_WINDOWEVENT_FOCUS_LOST:
			mKeyboardFocus = false;
			updateCaption = true;
			break;

			//Window minimized
		case SDL_WINDOWEVENT_MINIMIZED:
			mMinimized = true;
			break;

			//Window maxized
		case SDL_WINDOWEVENT_MAXIMIZED:
			mMinimized = false;
			break;

			//Window restored
		case SDL_WINDOWEVENT_RESTORED:
			mMinimized = false;
			break;
			//Hide on close
		case SDL_WINDOWEVENT_CLOSE:
			SDL_HideWindow(mWindow);
			break;
		}
	}
}

void LWindow::focus()
{
	//Restore window if needed
	if (!mShown)
	{
		SDL_ShowWindow(mWindow);
	}

	//Move window forward
	SDL_RaiseWindow(mWindow);
}

void LWindow::render()
{
	if (!mMinimized)
	{
		//Update screen
		SDL_RenderPresent(mRenderer);
	}
}

void LWindow::setWidth(int x)
{
	mWidth = x;
}
void LWindow::setHeight(int x)
{
	mHeight = x;
}
void LWindow::setFullscreen(bool x)
{
	mFullScreen = x;
}
int LWindow::getWidth()
{
	return mWidth;
}
int LWindow::getHeight()
{
	return mHeight;
}
SDL_Renderer* LWindow::getRenderer()
{
	return mRenderer;
}

void LWindow::free()
{
	SDL_DestroyWindow(mWindow);
}