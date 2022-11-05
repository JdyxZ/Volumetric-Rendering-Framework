#include "volumenode.h"

VolumeNode::VolumeNode(const char* volume_name)
{
	//Name
	name = volume_name;

	//Material
	volume_material = new VolumeMaterial();
	volume_material->volume_node = this;
	this->material = volume_material;

	//Mesh
	mesh = new Mesh();
	mesh->createCube();

	//Model
	updateModel();
}

VolumeNode::~VolumeNode()
{
	//delete(this);
}

void VolumeNode::updateModel()
{

	const float width = volume_material->current_volume->width * volume_material->current_volume->widthSpacing;
	const float height = volume_material->current_volume->height * volume_material->current_volume->heightSpacing;
	const float depth = volume_material->current_volume->depth * volume_material->current_volume->depthSpacing;
	this->model.setScale(1, height / width, depth / width);

}