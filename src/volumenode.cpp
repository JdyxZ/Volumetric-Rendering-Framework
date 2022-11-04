#include "volumenode.h"

VolumeNode::VolumeNode(const char* volume_name, const char* volume_dataset)
{
	//Name
	this->name = volume_name;

	//Material
	this->material = new StandardMaterial();

	//Shader
	this->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volumetric.fs");

	//Volume
	this->volume = new Volume();
	this->volume->load(volume_dataset);

	//Texture
	this->material->texture = new Texture();
	this->material->texture->create3DFromVolume(this->volume);

	//Mesh
	this->mesh = new Mesh();
	this->mesh->createCube();

	//Model
	const float width = volume->width * volume->widthSpacing;
	const float height = volume->height * volume->heightSpacing;
	const float depth = volume->depth * volume->depthSpacing;
	this->model.setScale(1, height/width, depth/width);

	//Sliders
	step_length = 0.0001;
	brightness = Vector3(1.0);
}

VolumeNode::~VolumeNode()
{
	//delete(this);
}

void VolumeNode::render(Camera* camera)
{
	if (material)
		material->render(mesh, model, camera, step_length, brightness);
}

void VolumeNode::renderInMenu()
{
	//Start with a typical scene node
	SceneNode::renderInMenu();

	//Add float sliders
	ImGui::SliderFloat("Step length", &step_length, 0.00000000000001, 0.5, "%.14f", 10.f);
	ImGui::SliderFloat3("Brightness", &brightness.x, 0.0, 10.0, "%.3f", 10.f);

}
