#pragma once

#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>
#include <sstream>
#include <SDL.h>
#include <mysql.h>
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
	bool serverDeleteEntry(string table, int ID);
	bool serverModifyTableStr(string table, int ID, string COL, string value);
	int serverGetRowCount(string table);
	bool copyDatabase(string table);
	bool serverInsertRow(string table, int posX, int posY, string color);
	bool attemptLogin(string user, string pass);
	bool setPlayerOnline();
	bool setPlayerOffline();
	int serverGetPlayersData();
	bool runQueryString(string);
	int attemptRegister(string user, string pass);
	bool insertProjectile(int, int, int, int);
	bool getProjectileData();
	
	void serverUpdatePlayerPosition(int, int);
	void setPlayersOnline(int);
	int getPlayersOnline();
	bool getPlayerSlotFree(int);
	void setPlayerSlotFree(int, bool);

	SDL_Rect getPlayerPos(int);

	string getClientID();
	string getClientNickname();
	string getPlayerNickname(int);

	string getServerName();
	string getServerIP();

	LPawn gPlayer[99];

protected:

private:
	bool runQuery(stringstream&);
	bool playerSlotFree[99];

	string pServerName;
	string pServerIP;

	int playersOnline;
	string playerID[99];
	string playerNickname[99];
	SDL_Rect playerPos[99];

	string clientID;
	string clientID2;
	string clientNickname;

	MYSQL* conn;
	MYSQL_ROW row;
	MYSQL_RES* res;
	
};
