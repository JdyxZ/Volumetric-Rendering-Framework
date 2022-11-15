//Uniforms
uniform vec3 u_camera_local_position;
uniform vec4 u_color;
uniform float u_step_length;
uniform float u_brightness;
uniform float u_alpha_cutoff;
uniform sampler3D u_texture;

uniform bool u_jittering;
uniform sampler2D u_blue_noise;
uniform bool u_tf_enabled;
uniform sampler2D u_tf;

//Interpolated
varying vec3 v_position;

//Constants
#define ITERATIONS 1000

float rand(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	//Initialize variables
	vec3 sample_position = v_position;
	vec3 step_vector = u_step_length * normalize(sample_position - u_camera_local_position);
	vec4 output_color = vec4(.0);

	// create the offset for the jittering implementation
	if(u_jittering)
	{
		// pseudo-random looking function implementation
		//float offset = rand(gl_FragCoord.xy);
		//sample_position += step_vector * rand(gl_FragCoord.xy);

		vec3 offset = texture2D(u_blue_noise, gl_FragCoord.xy / 128.0); // the noise texture has 128 pxls of width
		sample_position += step_vector * offset;
	}
	//Main loop
	for( int i = 0; i < ITERATIONS; ++i )
	{
		//Get texture coordinates
		vec3 texture_coordinates = (sample_position + vec3(1.0)) / 2.0;

		//Get density value
		float density = texture3D(u_texture, texture_coordinates).x;

		//Classification: Map color
		vec4 sample_color;

		if(u_tf_enabled)
		{
			sample_color = texture2D(u_tf, vec2(density, 0.5));
		}
		else
		{
			sample_color = vec4(density) * u_color;
		}
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
