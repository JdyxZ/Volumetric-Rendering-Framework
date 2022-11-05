//Uniforms
uniform vec3 u_camera_local_position;
uniform vec4 u_color;
uniform float u_step_length;
uniform float u_brightness;
uniform float u_alpha_cutoff;
uniform sampler3D u_texture;

//Interpolated
varying vec3 v_position;

//Constants
#define ITERATIONS 1000

void main()
{
	//Initialize variables
	vec3 sample_position = v_position;
	vec3 step_vector = u_step_length * normalize(sample_position - u_camera_local_position);
	vec4 output_color = vec4(.0);

	//Main loop
	for( int i = 0; i < ITERATIONS; ++i )
	{
		//Get texture coordinates
		vec3 texture_coordinates = (sample_position + vec3(1.0)) / 2.0;

		//Get density value
		float density = texture3D(u_texture, texture_coordinates).x;

		//Classification: Map color
		vec4 sample_color = vec4(density) * u_color;
		sample_color.rgb *= sample_color.a;

		//Composition: Accumulate color
		output_color += u_step_length * (1.0 - output_color.a) * sample_color;

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
	gl_FragColor = u_brightness * output_color;	
}
