#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"
#include "utils.h"
#include "volume.h"

#include <filesystem>
#include <map>

//Forward declaration
class VolumeNode;

//Namespace
using namespace std;

//Typedef
typedef std::map<std::string, std::pair<Volume*, Texture*>> loadedVolumes;

class Material {
public:

	Shader* shader = NULL;
	Texture* current_texture = NULL;
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera* camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera* cameras);
};

// TODO: Derived class VolumeMaterial
class VolumeMaterial : public Material
{
public:

	//Constructor and destructor
	VolumeMaterial();
	~VolumeMaterial();

	//Attributes
	VolumeNode* volume_node = NULL;
	Volume* current_volume = NULL;
	string current_volume_key;
	loadedVolumes volumes;
	Matrix44 inverse_model;
	float step_length;
	float brightness;

	//Inherited methods
	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();

	//Class methods
	void loadVolumes();
};

#endif