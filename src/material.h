#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"
#include "volume.h"

#include <filesystem>
#include <map>

//Forward declaration
class VolumeNode;

//Namespace
using namespace std;

//Typedef
typedef std::map<std::string, std::pair<Volume*, Texture*>> loadedVolumes;

//Jittering types
enum Jittering
{
	NoiseTexture,
	RandomGenerator
};

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

	//Main attributes
	VolumeNode* volume_node = NULL;
	Matrix44 inverse_model;
	float step_length;
	float brightness;
	float alpha_cutoff;
	float density_threshold;

	//Volume loader
	Volume* current_volume = NULL;
	string current_volume_key;
	loadedVolumes volumes;

	//Jittering
	Texture* blue_noise;
	bool jittering;
	Jittering jittering_type;

	//Transfer function
	bool transfer_function;
	int current_transfer_texture;
	float alpha_factor;
	Texture* transfer_textures = NULL;
	map<int, string> transfer_table;

	//Volume clipping
	bool volume_clipping;
	Vector4 plane_parameters;

	//Isosurfaces
	bool isosurfaces;
	float isosurface_threshold;
	float h;

	//Phong
	bool phong;
	Vector3 light_direction;

	//Inherited methods
	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();

	//Class methods
	void loadVolumes();
	void loadTransferTextures();
};

#endif