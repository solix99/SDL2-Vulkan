#pragma once

#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>
#include <sstream>
#include <SDL.h>
#include "LWindow.h"
#include <SDL_thread.h>
#include "LPawn.h"
#include "LProjectile.h"
#include <Windows.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

using namespace std;

class LServer
{
public:
	LServer();

	LPawn gPlayer[99];

protected:


	/*
	MYSQL* conn;
	MYSQL_ROW row;
	MYSQL_RES* res;
	*/
};
