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
	blue_noise->Texture::Get("data/images/blueNoise.png");

	//Jittering
	jittering = false;
	jittering_type = NoiseTexture;

	//Transfer function
	transfer_function = false;
	current_transfer_texture = 0;
	alpha_factor = 1.f;
	loadTransferTextures();

	//Volume clipping
	volume_clipping = false;
	plane_parameters = Vector4(0.f, 0.25, 0.f, -0.09);

	//Isosurfaces
	isosurfaces = false;
	isosurface_threshold = 0.5;
	h = 0.02;

	//Phong
	phong = false;
	light_direction = Vector3(0.f, 0.f, 1.f);
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
	shader->setUniform("u_alpha_factor", alpha_factor);
	shader->setUniform("u_volume_clipping", volume_clipping);
	shader->setUniform("u_plane_parameters", plane_parameters);
	shader->setUniform("u_isosurfaces", isosurfaces);
	shader->setUniform("u_isosurface_threshold", isosurface_threshold);
	shader->setUniform("u_h", h);
	shader->setUniform("u_phong", phong);
	shader->setUniform("u_light_direction", light_direction);

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
	if (transfer_textures != NULL)
	{
		texture_slot++;
		shader->setTexture("u_transfer_textures", transfer_textures, texture_slot);
	}
		
}

void VolumeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//Enable OpenGl flags
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();

		//Disable OpenGl flags
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
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
	if(isosurfaces) ImGui::SliderFloat("Density threshold", &density_threshold, isosurface_threshold + 0.05, 1.f, "%.4f", 10.f);
	else ImGui::SliderFloat("Density threshold", &density_threshold, 0.0001, 1.f, "%.4f", 10.f);

	//Jittering
	ImGui::Checkbox("Jittering", &jittering);
	if(jittering) ImGui::Combo("Type", (int*)&jittering_type, "Noise texture\0Random generator", 2);

	//Transfer function
	if(transfer_textures != NULL) ImGui::Checkbox("Transfer function", &transfer_function);
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
		ImGui::SliderFloat("Alpha factor", &alpha_factor, 0.0001, 100.f, "%.4f", 10.f);
	}

	//Volume clipping
	ImGui::Checkbox("Volume clipping", &volume_clipping);
	if (volume_clipping) ImGui::SliderFloat4("Plane Parameters", &plane_parameters.x, -2.f, 2.f, "%.6f");

	//Isosurfaces
	ImGui::Checkbox("Isosurfaces", &isosurfaces);
	if (isosurfaces)
	{
		ImGui::SliderFloat("Isosurface threshold", &isosurface_threshold, 0.000001, density_threshold + 0.05, "%.6f");
		ImGui::SliderFloat("H value", &h, 0.0001, 100.f, "%.4f", 10.f);
		ImGui::Checkbox("Apply Phong", &phong);
		if(phong) ImGui::SliderFloat3("Light direction", &light_direction.x, 0.f, 10.f, "%.3f", 10.f);
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
	//Check if transfer textures has already been loaded
	const char* storage_name = "transfer_texture";
	Texture* storage_texture = Texture::onlyGet(storage_name);
	if (storage_texture != NULL) return;

	//Declare varaibles
	string current_path = filesystem::current_path().string();
	string images_path = current_path + "\\data\\images";
	vector<string> filepaths;
	string texture_name;
	int texture_position = 0;

	//Compute transfer textures vector
	for (const auto& entry : filesystem::directory_iterator(images_path))
	{
		//Get image
		string full_path = entry.path().string();
		string simplified_path = full_path.substr(full_path.find("data\\images"), full_path.size());
		std::replace(simplified_path.begin(), simplified_path.end(), '\\', '/');
		string image = full_path.substr(images_path.size() + 1, full_path.size());

		//Get extension position
		size_t ext_pos = image.find(".");

		//Check image
		if (ext_pos != string::npos && image.find("TT") != string::npos)
		{
			//Save filepath
			filepaths.push_back(simplified_path);

			//Append image position and name to table
			texture_name = image.substr(0, ext_pos);
			transfer_table.emplace(make_pair(texture_position, texture_name));
			texture_position++;
			assert(texture_position < MAX_TRANSFER_TEXTURES && "There are more the MAX_TRANSFER_TEXTURES images to convert. Change MAX_TRANSFER_TEXTURES or delete images");
		}
			
	}

	//Create and upload array of transfer textures
	if (filepaths.empty())
	{
		std::cout << "Error: No images have been found with prefix TT. Texture array will be NULL" << std::endl;
		transfer_textures = NULL;
		return;
	}
	transfer_textures = new Texture();
	bool success = transfer_textures->uploadTextureArray(filepaths, storage_name);

	//Check that texture is right
	if (!success)
	{
		transfer_textures = NULL;
	}

}