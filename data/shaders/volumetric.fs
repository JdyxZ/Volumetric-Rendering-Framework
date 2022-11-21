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
				//Classification: Map color
				vec4 color_sample = map_color(density);

				//Composition: Accumulate color
				output_color += u_step_length * (1.0 - output_color.a) * color_sample;
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
	if(output_color.a < u_alpha_cutoff) discard;

	//Output
	FragColor = u_brightness * output_color;	
}
