#include "Object.h"


Object::Object()
{

}

void Object::setPosition(glm::vec3 POS)
{
	POSITION = POS;
}
glm::vec3 Object::getPosition()
{
	return POSITION;
}

void Object::setMesh(Mesh* msh)
{
	meshPtr = msh;
}

void Object::init(glm::vec3 POS, Mesh* msh)
{
	POSITION = POS;
	meshPtr = msh;
}


Mesh* Object::getMesh()
{
	return meshPtr;
}