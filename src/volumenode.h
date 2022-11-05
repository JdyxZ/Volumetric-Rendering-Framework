#pragma once
#include "scenenode.h"

class VolumeNode : public SceneNode
{
public:
	//Constructor and destructor
	VolumeNode(const char* volume_name);
	~VolumeNode();
	
	//Auxiliar pointer
	VolumeMaterial* volume_material = NULL;

	//Class methods
	void updateModel();
};

