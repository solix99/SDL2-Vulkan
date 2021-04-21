#include "LServer.h"

LServer::LServer()
{
	conn = mysql_init(0);
	//conn = mysql_real_connect(conn, "remotemysql.com", "qCVqUCrDjk", "GeAUGSYSUL", "qCVqUCrDjk", 0, NULL, 0);
	conn = mysql_real_connect(conn,"192.168.1.6","admin2","admin2","game_db",3307,NULL,0);
	//conn = mysql_real_connect(conn,"192.168.1.14","admin2","admin2","phpmyadmin",0,NULL,0);
	//conn = mysql_real_connect(conn,"206.72.206.123","solix","LQeSx52bdH5iwX6","game",0,NULL,0);

	if (!conn)
	{
		cout << endl << "SQL SERVER CONNECTION FAILED";
	}

	for (unsigned int i = 0; i < 99; i++)
		playerSlotFree[i] = true;

	playersOnline = 0;


}

bool LServer::insertProjectile(int x, int y, int dx, int dy)
{
	stringstream ss;
	ss << "INSERT INTO projectile (ID,posX,posY,posDX,posDY) VALUES ('" << clientID << "','" << x << "','" << y << "','" << dx << "','" << dy << "');";

	if (!runQuery(ss))
	{
		cout << endl << "Failed server query,spawnPlayerProjectile";
		return false;
	}

	return true;
}

bool LServer::serverDeleteEntry(string table, int ID)
{
	int qstate = 0;

	stringstream ss;
	string query;

	ss << "DELETE FROM " << table << " WHERE ID=" << ID << ";";
	cout << ss.str() << endl;
	query = ss.str();
	const char* q = query.c_str();

	qstate = mysql_query(conn, q);

	if (qstate != 0)
	{
		cout << endl << "FAILED TO DELETE ENTRY";
		return false;
	}
}

string LServer::getPlayerNickname(int i)
{
	return playerNickname[i];
}

bool LServer::getProjectileData()

{
	{
	if (!runQueryString("SELECT * FROM projectile;"))
	{
		cout << endl << " Failed getProjectileData, SELECT * FROM projectile;";
		return false;
	}

	stringstream ss;

	bool found = false;
	res = mysql_store_result(conn);

	//while (row = mysql_fetch_row(res))
	//
	//	for (unsigned int i = 0; i < playersOnline + 5; i++)
	//	{
	//		if (stoi(row[0]) == gPlayer[i].getPlayerID())
	//		{
	//			for (int j = 0; j < 99; j++)
	//			{
	//				if (gPlayer[i].gProjectile[j].getSlotFree())
	//				{
	//					gPlayer[i].gProjectile[j].setPosX(stoi(row[1]));
	//					gPlayer[i].gProjectile[j].setPosY(stoi(row[2]));
	//					gPlayer[i].gProjectile[j].setDestX(stoi(row[3]));
	//					gPlayer[i].gProjectile[j].setDestY(stoi(row[4]));
	//					gPlayer[i].gProjectile[j].setAngle(90 + (atan2(stoi(row[4]) - stoi(row[2]), stoi(row[3]) - stoi(row[1])) * 180 / 3.14f));
	//					gPlayer[i].gProjectile[j].setSlotFree(false);
	//
	//					break;
	//				}
	//			}
	//			found = true;
	//			break;
	//		}
	//	}
	//	if (!found)
	//	{
	//		for (unsigned int i = 0; i < playersOnline + 5; i++)
	//		{
	//			if (!gPlayer[i].getIfSlotUsed())
	//			{
	//				gPlayer[i].setPlayerID((stoi(row[0])));
	//
	//				for (int j = 0; j < 99; j++)
	//				{
	//					if (gPlayer[i].gProjectile[j].getSlotFree())
	//					{
	//						gPlayer[i].gProjectile[j].setPosX(stoi(row[1]));
	//						gPlayer[i].gProjectile[j].setPosY(stoi(row[2]));
	//						gPlayer[i].gProjectile[j].setDestX(stoi(row[3]));
	//						gPlayer[i].gProjectile[j].setDestY(stoi(row[4]));
	//						gPlayer[i].gProjectile[j].setAngle(90 + (atan2(stoi(row[4]) - stoi(row[2]), stoi(row[3]) - stoi(row[1])) * 180 / 3.14f));
	//						gPlayer[i].gProjectile[j].setSlotFree(false);
	//
	//						break;
	//					}
	//				}
	//				gPlayer[i].setIfSlotUsed(true);
	//				break;
	//			}
	//		}
	//	}
	//
		ss.str("");
		ss << "DELETE FROM projectile WHERE ID = '" << row[0] << "';";

		if (!runQuery(ss))
		{
			cout << endl << "Failed runQuery, getProjectileData,DELETE FROM projectile WHERE ID = ";
			return false;
		}

		found = false;

	}

	return true;
}

bool LServer::serverModifyTableStr(string table, int ID, string COL, string value)
{
	stringstream ss;
	string query;
	const char* q;

	int qstate = 0;

	ss << "UPDATE " << table << " SET " << COL << " = '" << value << "' WHERE ID=" << ID << ";";
	query = ss.str();
	q = query.c_str();

	qstate = mysql_query(conn, q);

	if (qstate == 0)
	{
		cout << endl << "Table updated succesfully";
	}
	else
	{
		cout << endl << "Table failed to update.";
		return false;
	}

	return true;
}

int LServer::serverGetRowCount(string table)
{
	int qstate = 0;

	stringstream ss;
	string query;

	ss << "SELECT Count(*) FROM INFORMATION_SCHEMA.Columns where TABLE_NAME = '" << table << "';";
	query = ss.str();
	const char* q = query.c_str();

	qstate = mysql_query(conn, q);

	if (!qstate)
	{
		res = mysql_store_result(conn);
		row = mysql_fetch_row(res);
		//cout<<endl<<row[0];
		return atoi(row[0]);
	}
	else
	{
		cout << endl << "Invalid SQL Syntax" << table;
	}
	mysql_free_result(res);
}

bool LServer::copyDatabase(string table)
{
	stringstream ss;
	string query;
	const char* q;

	string data[999];

	ss << "SELECT * FROM " << table << ";";
	query = ss.str();
	q = query.c_str();

	int qstate = mysql_query(conn, q);

	if (qstate == 0)
	{
		if (!qstate)
		{
			res = mysql_store_result(conn);

			while (row = mysql_fetch_row(res))
			{
				cout << row[0] << " " << row[1] << " " << row[2] << " " << row[3] << endl;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


bool LServer::setPlayerOffline()
{
	stringstream query;

	query << "DELETE FROM online_ply WHERE ID = '" << clientID << "';";

	if (!runQuery(query))
	{
		return false;
	}

	return true;
}

string LServer::getClientNickname()
{
	return clientNickname;
}

bool LServer::setPlayerOnline()
{
	stringstream query;

	query << "INSERT INTO online_ply(ID,name,charX,charY) values ('" << clientID << "','" << clientNickname << "','0','0');";

	if (!runQuery(query))
	{
		return false;
	}


	return true;

}

bool LServer::attemptLogin(string user, string pass)
{
	stringstream ss;
	string query;
	const char* q;

	ss << "SELECT * FROM account_db;";
	query = ss.str();
	q = query.c_str();

	int qstate = mysql_query(conn, q);

	if (qstate == 0)
	{
		if (!qstate)
		{
			res = mysql_store_result(conn);

			while (row = mysql_fetch_row(res))
			{
				if (strcmp(user.c_str(), row[1]) == 0 && strcmp(pass.c_str(), row[2]) == 0)
				{
					clientID = row[0];
					clientID2 = row[0];
					clientNickname = row[1];
					return true;
				}
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return false;
}

string LServer::getClientID()
{
	return clientID;
}

int LServer::attemptRegister(string user, string pass)
{

	stringstream ss;
	string query;
	const char* q;


	ss << "SELECT * FROM account_db;";
	query = ss.str();
	q = query.c_str();


	if (user.length() > 3 && pass.length() > 3)
	{
		int qstate = mysql_query(conn, q);

		if (qstate == 0)
		{
			if (!qstate)
			{
				res = mysql_store_result(conn);

				while (row = mysql_fetch_row(res))
				{
					if (strcmp(user.c_str(), row[1]) == 0)
					{
						return 1;
					}
				}
			}
			else
			{
				cout << endl << "Server error";
				return 2;
			}
		}
		else
		{
			cout << endl << "Server error";
			return 2;
		}

		ss.str("");
		ss << "INSERT INTO " << "account_db (USER,PASS,LEVEL) values ('" << user << "','" << pass << "','1');";
		cout << endl << ss.str();
		query = ss.str();
		q = query.c_str();

		qstate = mysql_query(conn, q);

		if (qstate == 0)
		{
			if (qstate)
			{
				cout << endl << "Server error";
				return 2;
			}
			else
			{

			}
		}
		else
		{
			cout << endl << "Server error";
			return 2;
		}

		return 0;
	}
	else
	{
		return 3;
	}


	//1 - username already exists
	//2 - server error
	//3 - invalid data
	//0 - succesful
}

bool LServer::serverInsertRow(string table, int posX, int posY, string color)
{

	stringstream ss;
	int qstate = 0;

	ss << "INSERT INTO " << table << " (posX,posY,color) values ('" << posX << "','" << posY << "','" << color << "');";
	string query = ss.str();
	cout << endl << query;
	const char* q = query.c_str();

	qstate = mysql_query(conn, q);

	if (qstate == 0)
	{
		cout << endl << "Values inserted";
	}
	else
	{
		cout << endl << "Values failed to insert";
	}
	return true;
}

string LServer::getServerName()
{
	return pServerName;
}
string LServer::getServerIP()
{
	return pServerIP;
}

int LServer::serverGetPlayersData()
{
	if (!runQueryString("SELECT * FROM online_ply;"))
	{
		cout << endl << "FAILED runQueryString ,serverGetPlayersData";
	}

	playersOnline = 0;

	res = mysql_store_result(conn);

	while (row = mysql_fetch_row(res))
	{
		if (clientID != row[0])
		{
			playerNickname[playersOnline] = row[1];
			playerPos[playersOnline].x = stoi(row[2]);
			playerPos[playersOnline].y = stoi(row[3]);
			playersOnline++;
		}

	}
	return true;
}
bool LServer::getPlayerSlotFree(int i)
{
	return playerSlotFree[i];
}
void LServer::setPlayerSlotFree(int i, bool b)
{
	playerSlotFree[i] = b;
}

bool LServer::runQuery(stringstream& ssq)
{
	string query;
	const char* q;

	query = ssq.str();
	q = query.c_str();

	int qstate = mysql_query(conn, q);

	if (qstate != 0)
	{
		return false;
	}

	return true;
}

bool LServer::runQueryString(string qu)
{
	string query;
	const char* q;

	q = qu.c_str();

	int qstate = mysql_query(conn, q);

	if (qstate != 0)
	{
		return false;
	}

	return true;

}

void LServer::setPlayersOnline(int k)
{
	playersOnline = k;
}
int LServer::getPlayersOnline()
{
	return playersOnline;
}

void LServer::serverUpdatePlayerPosition(int x, int y)
{
	stringstream ss;

	ss << "UPDATE online_ply SET charX ='" << x << "' , charY = '" << y << "' WHERE ID='" << clientID << "';";

	if (!runQuery(ss))
	{
		cout << endl << "Failed runQuery ,serverUpdatePlayerPosition";
	}
}





SDL_Rect LServer::getPlayerPos(int i)
{
	return playerPos[i];
}
