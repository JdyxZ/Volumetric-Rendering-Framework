//Uniforms
uniform vec4 u_color;
uniform vec3 u_camera_position;
uniform mat4 u_inverse_model;
uniform float u_step_length;
uniform float u_brightness;
uniform sampler3D u_texture;

//Interpolated
varying vec3 v_position;

//Constants
#define ITERATIONS 1000

void main()
{
	//Initialize variables
	vec3 sample_position = v_position;
	vec3 local_camera_position = (u_inverse_model * vec4(u_camera_position, 1.0)).xyz;
	vec3 step_vector = normalize(v_position - local_camera_position) * u_step_length;
	vec3 texture_coordinates = vec3(.0);
	float density = 0.0;
	vec4 sample_color = vec4(.0);
	vec4 output_color = vec4(.0);
	bool position_in_range = false;

	//Main loop
	for( int i = 0; i < ITERATIONS; ++i )
	{
		//Get texture coordinates
		texture_coordinates = (sample_position + vec3(1.0)) / 2.0;

		//Get density value
		density = texture3D(u_texture, texture_coordinates).x;

		//Classification: Map color
		sample_color = vec4(density, density, density, density);
		sample_color.rgb *= sample_color.a;

		//Composition: Accumulate color
		output_color += u_step_length * (1.0 - output_color.a) * sample_color;

		//Update position
		sample_position += step_vector;

		//Update auxiliar variable
		position_in_range = sample_position.x < 1.0 && sample_position.y < 1.0 && sample_position.z < 1.0 && sample_position.x > -1.0 && sample_position.y > -1.0 && sample_position.z > -1.0;

		//Termination
		if(!position_in_range) //out of range
		{
			break;
		}
		else if(output_color.a >= 1.0)
		{
			break;
		}
		else if(output_color.a < 0.01)
		{
			discard;
		}

	}

	//Output
	gl_FragColor = vec4(u_brightness, 1.0) * output_color;
	//gl_FragColor = vec4(u_brightness, 1.0) * u_color * output_color;
}
