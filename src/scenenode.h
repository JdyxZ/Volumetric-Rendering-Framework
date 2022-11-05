#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"

//Use prefix std
using namespace std;

//Forward declaration
class Light;

class SceneNode {
public:

	//Constructors and desctructors
	SceneNode();
	SceneNode(const char* name);
	~SceneNode();

	//Attributes
	string name;
	Material * material = NULL;
	Mesh* mesh = NULL;
	Matrix44 model;
	static unsigned int lastNameId;

	//Virtual methods
	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};

// TODO: Derived class VolumeNode

#endif