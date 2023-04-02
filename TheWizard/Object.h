#pragma once

#include "Mesh.h"

using namespace std;


class Object
{
public:
	Object();

	void setPosition(glm::vec3 POS);
	glm::vec3 getPosition();
	void setMesh(Mesh *msh);
	void init(glm::vec3 POS, Mesh* msh);
	void setObjectName(string name);
	string getObjectName();

private:
	
	glm::vec3 POSITION{0,0,0};
	Mesh* meshPtr = nullptr;
	string OBJECT_NAME;

};

