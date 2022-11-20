#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

constexpr int MAX_TRANSFER_TEXTURES = 8; //Bear in mind the total number of slots must be less than 16 (change this value in the shader and here)

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", color);

	if (current_texture)
		shader->setUniform("u_texture", current_texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

VolumeMaterial::VolumeMaterial()
{
	//Parameters
	color = vec4(1.f, 1.f, 1.f, 1.f);
	step_length = 0.01;
	brightness = 15.f;
	alpha_cutoff = 0.05;
	density_threshold = 1.f;

	//Shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volumetric.fs");

	//Load volumes
	if (volumes.empty()) loadVolumes();
	current_volume_key = volumes.begin()->first;

	//Volume
	current_volume = volumes[current_volume_key].first;

	//Texture
	current_texture = volumes[current_volume_key].second;

	//Noise texture
	blue_noise = new Texture();
	blue_noise->load("data/images/blueNoise.png");

	//Jittering
	jittering = false;
	jittering_type = NoiseTexture;

	//Transfer function
	transfer_function = false;
	current_transfer_texture = 0;
	if (transfer_textures.empty()) loadTransferTextures();
}

VolumeMaterial::~VolumeMaterial()
{
	//delete(this)
}

void VolumeMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//Upload volume node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_model", model);
	shader->setUniform("u_camera_local_position", inverse_model * camera->eye);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", color);
	shader->setUniform("u_step_length", step_length);
	shader->setUniform("u_brightness", brightness);
	shader->setUniform("u_alpha_cutoff", alpha_cutoff);
	shader->setUniform("u_density_threshold", density_threshold);
	shader->setUniform("u_jittering", jittering);
	shader->setUniform("u_jittering_type", jittering_type);
	shader->setUniform("u_transfer_function", transfer_function);
	shader->setUniform("u_current_transfer_texture", current_transfer_texture);

	//Textures
	int texture_slot = 0;
	if (current_texture)
	{
		shader->setUniform("u_texture", current_texture, texture_slot);
	}
	if (blue_noise)
	{
		texture_slot++;
		shader->setUniform("u_blue_noise_width", (float)blue_noise->width);
		shader->setTexture("u_blue_noise", blue_noise, texture_slot);
	}
	if (!transfer_textures.empty())
	{
		for (auto it = transfer_textures.begin(); it != transfer_textures.end(); ++it)
		{
			texture_slot++;
			shader->setTexture("u_transfer_textures", (* it), texture_slot);
		}
		
	}
		
}

void VolumeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void VolumeMaterial::renderInMenu()
{
	//Volume menu
	if (ImGui::BeginCombo("Volume", current_volume_key.c_str()))
	{
		for (auto it = volumes.begin(); it != volumes.end(); ++it)
		{
			bool is_selected = (current_volume_key == it->first);
			if (ImGui::Selectable(it->first.c_str(), is_selected))
			{
				current_volume_key = it->first;
				current_volume = it->second.first;
				current_texture = it->second.second;
				volume_node->updateModel();
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	//Main attributes
	ImGui::SliderFloat3("Color", &color.x, 0.0, 1.0, "%.3f", 10.f);
	ImGui::SliderFloat("Step length", &step_length, 0.000001, 50.f, "%.6f", 10.f);
	ImGui::SliderFloat("Brightness", &brightness, 0.0, 50.0, "%.3f", 10.f);
	ImGui::SliderFloat("Alpha cutoff", &alpha_cutoff, 0.00000001, 1.f, "%.8f", 10.f);
	ImGui::SliderFloat("Density threshold", &density_threshold, 0.0001, 1.f, "%.4f", 10.f);

	//Jittering
	ImGui::Checkbox("Jittering", &jittering);
	if(jittering) ImGui::Combo("Type", (int*)&jittering_type, "Noise texture\0Random generator", 2);

	//Transfer function
	if(!transfer_textures.empty()) ImGui::Checkbox("Transfer function", &transfer_function);
	
	if (transfer_function)
	{
		if (ImGui::BeginCombo("Transfer Texture", transfer_table[current_transfer_texture].c_str()))
		{
			for (auto it = transfer_table.begin(); it != transfer_table.end(); ++it)
			{

				bool is_selected = (current_transfer_texture == it->first);
				if (ImGui::Selectable(it->second.c_str(), is_selected))
					current_transfer_texture = it->first;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
}

void VolumeMaterial::loadVolumes()
{
	//Declare varaibles
	string current_path = filesystem::current_path().string();
	string volumes_path = current_path + "\\data\\volumes";

	//Compute volumes vector
	for (const auto& entry : filesystem::directory_iterator(volumes_path))
	{
		//Get volume
		string str_path = entry.path().string();
		string volume_key = str_path.substr(volumes_path.size() + 1, str_path.size());

		//Check volume
		if (volume_key.find(".") != string::npos)
		{
			//Create and load new volume
			Volume* new_volume = new Volume();
			bool success = new_volume->load(("data/volumes/" + volume_key).c_str());

			//Append volume and texture if it is right
			if (success)
			{
				//Create and load new texture
				Texture* new_texture = new Texture();
				new_texture->create3DFromVolume(new_volume);

				//Emplace values
				volumes.emplace(volume_key, make_pair(new_volume, new_texture));
			}
			else
			{
				delete new_volume;
			}

		}
	}
}

void VolumeMaterial::loadTransferTextures()
{
	//Declare varaibles
	string current_path = filesystem::current_path().string();
	string images_path = current_path + "\\data\\images";
	string texture_name = "error";
	int texture_position = 0;

	//Compute transfer textures vector
	for (const auto& entry : filesystem::directory_iterator(images_path))
	{
		//Get image
		string str_path = entry.path().string();
		string image = str_path.substr(images_path.size() + 1, str_path.size());

		//Get extension position
		size_t ext_pos = image.find(".");

		//Check image
		if (ext_pos != string::npos && image.find("TT") != string::npos)
		{
			//Create and load new texture
			Texture* new_texture = new Texture();
			bool success = new_texture->load(("data/images/" + image).c_str());

			//Append texture to the vector if it is right
			if (success)
			{
				texture_name = image.substr(0, ext_pos);
				transfer_textures.push_back(new_texture);
				transfer_table.emplace(make_pair(texture_position, texture_name));
				texture_position++;
				if (texture_position > MAX_TRANSFER_TEXTURES) 
					return;
			}
			else
			{
				delete new_texture;
			}

		}
	}
}