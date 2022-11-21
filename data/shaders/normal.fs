in vec3 v_position;
in vec3 v_world_position;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

void main()
{
	gl_FragColor = vec4(v_normal,1.0);
}
