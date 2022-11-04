#pragma once
#include "scenenode.h"
#include "volume.h"
#include "texture.h"

class VolumeNode : public SceneNode
{
public:
	//Constructor and destructor
	VolumeNode(const char* volume_name, const char* volume_dataset);
	~VolumeNode();

	//Atributes
	float step_length;
	Vector3 brightness;
	Volume* volume = NULL;

	//Inherited methods
	virtual void render(Camera* camera);
	virtual void renderInMenu();
};

