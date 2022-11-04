#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model, float step_length, Vector3 brightness) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera* camera, float step_length = 0.f, Vector3 brightness = Vector3(1.f)) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model, float step_length, Vector3 brightness);
	void render(Mesh* mesh, Matrix44 model, Camera * camera, float step_length = 0.f, Vector3 brightness = Vector3(1.f));
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera* camera, float step_length = 0.f, Vector3 brightness = Vector3(1.f));
};

// TODO: Derived class VolumeMaterial

#endif