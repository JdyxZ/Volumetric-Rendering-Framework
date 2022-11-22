//Constants
const int MAX_TRANSFER_TEXTURES = 8; //Bear in mind the total number of slots must be less than 16 (change this value here and in material.cpp)

//Uniforms
uniform vec3 u_camera_local_position;
uniform vec4 u_color;
uniform float u_step_length;
uniform float u_brightness;
uniform float u_alpha_cutoff;
uniform float u_density_threshold;
uniform sampler3D u_texture;
uniform sampler2D u_blue_noise;
uniform float u_blue_noise_width;
uniform bool u_jittering;
uniform int u_jittering_type;
uniform bool u_transfer_function;
uniform int u_current_transfer_texture;
uniform float u_alpha_factor;
uniform sampler2DArray u_transfer_textures;
uniform bool u_volume_clipping;
uniform vec4 u_plane_parameters;
uniform bool u_isosurfaces;
uniform float u_isosurface_threshold;
uniform float u_h;
uniform bool u_phong;
uniform vec3 u_light_direction;

//Interpolated
in vec3 v_position;

//Output
out vec4 FragColor;

//Constants
#define ITERATIONS 1000

//Jiterring
vec3 compute_offset()
{
	if(u_jittering)
	{
		//Noise texture
		if(u_jittering_type == 1)
		{
			return texture(u_blue_noise, gl_FragCoord.xy / u_blue_noise_width).rgb; 
		}
		//Pseudo-random generator
		else
		{
			return vec3(fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453));
		}
	}
	return vec3(.0);
}

//Transfer function
vec4 map_color(const float density)
{
	vec4 color_sample;
	if(u_transfer_function)
	{
		color_sample = texture(u_transfer_textures, vec3(density, 1, u_current_transfer_texture));
		color_sample.a = clamp(color_sample.a * density * u_alpha_factor, 0.0, 1.0); 
	}
	else
	{
		color_sample = vec4(density) * u_color;
		color_sample.rgb *= color_sample.a;
	}

	return color_sample;
}

//Volume clipping
bool check_clipping(vec3 pos)
{
	if(u_volume_clipping)
	{
		//Auxiliar var
		float equation_result = u_plane_parameters.x * pos.x + u_plane_parameters.y * pos.y +  u_plane_parameters.z * pos.z +  u_plane_parameters.w;

		//Check plane clipping
		if(equation_result > 0) return true;
		else return false;
	}

	return false;
}

//Normal vector
vec3 compute_normal(in vec3 coord)
{
	//Compute gradient
	float row_x = texture(u_texture, vec3(coord.x + u_h, coord.y, coord.z)).x - texture(u_texture, vec3(coord.x - u_h, coord.y, coord.z)).x;
	float row_y = texture(u_texture, vec3(coord.x, coord.y + u_h, coord.z)).x - texture(u_texture, vec3(coord.x, coord.y - u_h, coord.z)).x;
	float row_z = texture(u_texture, vec3(coord.x, coord.y, coord.z + u_h)).x - texture(u_texture, vec3(coord.x, coord.y, coord.z - u_h)).x;
	vec3 gradient = 1/(2*u_h) * vec3(row_x, row_y, row_z);

	//Compute normal vector from gradient
	vec3 normal_vector = normalize(-gradient);

	//Return normal vector
	return normal_vector;
}

//Illumination model
void Phong(inout vec4 color_sample, in vec3 sample_position, in vec3 texture_coordinates)
{
	//Compute necessary vectors
	vec3 L = normalize(u_light_direction);
	vec3 N = compute_normal(texture_coordinates);
	vec3 V = normalize(u_camera_local_position - sample_position);
	vec3 R = normalize(reflect(-L, N));

	//Compute dot products
	float NdotL = clamp(dot(N,L), 0.0, 1.0);
    float RdotV = clamp(dot(R,V), 0.0, 1.0);

	//Set color: We make some assumptions
	color_sample.rgb *= (NdotL + pow(RdotV, 10.0));
	color_sample.a = 1;
}

//Main
void main()
{
	//Initialize variables
	vec3 offset = compute_offset();
	vec3 step_vector = u_step_length * normalize(v_position - u_camera_local_position);
	vec3 sample_position = v_position + step_vector * offset;
	vec4 output_color = vec4(.0);

	//Main loop
	for( int i = 0; i < ITERATIONS; ++i )
	{
		//Check plane clipping
		if(!check_clipping(sample_position))
		{
			//Get texture coordinates
			vec3 texture_coordinates = (sample_position + vec3(1.0)) / 2.0;

			//Get density value
			float density = texture(u_texture, texture_coordinates).x;

			//Apply density threshold
			if(density <= u_density_threshold)
			{
				//Isosurfaces
				if(u_isosurfaces)
				{
					//Only values higher than the threshold are candidates to be equal to isosurface_density
					if(density > u_isosurface_threshold)
					{
						//Classification: Map color
						vec4 color_sample = map_color(density);

						//Compute and apply illumination model
						if(u_phong) Phong(color_sample, sample_position, texture_coordinates);					

						//No composition: Final color will be phong's result
						output_color = color_sample;
					}
					//"First" schema: Break the loop
					//break;
				}
				else
				{
					//Classification: Map color
					vec4 color_sample = map_color(density);

					//Composition: Accumulate color
					output_color += u_step_length * (1.0 - output_color.a) * color_sample;
				}
			}
		}

		//Update position
		sample_position += step_vector;

		//Update auxiliar variable
		bool position_in_range = sample_position.x < 1.0 && sample_position.y < 1.0 && sample_position.z < 1.0 && sample_position.x > -1.0 && sample_position.y > -1.0 && sample_position.z > -1.0;

		//Early termination
		if(output_color.a >= 1.0) break;
		else if(!position_in_range) break;
	}

	//Wipe black pixels
	if(!u_isosurfaces && output_color.a < u_alpha_cutoff) discard;

	//Output
	FragColor = u_brightness * output_color;	
}
