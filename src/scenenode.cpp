#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int mesh_selected = 0;

SceneNode::SceneNode()
{
	this->name = string("Scene Node" + to_string(lastNameId++));
}

SceneNode::SceneNode(const char * name)
{
	this->name = name;
}

SceneNode::~SceneNode()
{

}

void SceneNode::render(Camera* camera)
{	
	if (material)
		material->render(mesh, model, camera);
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
		
		ImGui::TreePop();
	}

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		material->renderInMenu();
		ImGui::TreePop();
	}

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0");

		ImGui::TreePop();
	}
}

VolumeNode::VolumeNode(const char* volume_name)
{
	//Name
	name = volume_name;

	//Material
	volume_material = new VolumeMaterial();
	volume_material->volume_node = this;
	material = volume_material;

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
	//Model
	const float width = volume_material->current_volume->width * volume_material->current_volume->widthSpacing;
	const float height = volume_material->current_volume->height * volume_material->current_volume->heightSpacing;
	const float depth = volume_material->current_volume->depth * volume_material->current_volume->depthSpacing;
	this->model.setScale(width/400, height / 400, depth / 400);

	//Inverse model
	volume_material->inverse_model = model;
	volume_material->inverse_model.inverse();

		std::cout << "widthSpacing: " << volume_material->current_volume->widthSpacing << '\n';
	std::cout << "heigthSpacing: " << volume_material->current_volume->heightSpacing << '\n';
	std::cout << "depthSpacing: " << volume_material->current_volume->depthSpacing << '\n';

	std::cout << "width" << volume_material->current_volume->width;
	std::cout << "height" << volume_material->current_volume->height;
	std::cout << "depth" << volume_material->current_volume->depth;
}
