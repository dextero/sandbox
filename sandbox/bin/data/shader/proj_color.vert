uniform mat4 u_matViewProjection;
uniform mat4 u_matModel;

attribute vec3 a_vertex;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
	v_color = a_color;
	gl_Position = u_matViewProjection * u_matModel * vec4(a_vertex, 1.0);
}
