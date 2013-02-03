uniform mat4 u_matViewProjection;
uniform mat4 u_matModel;

attribute vec3 a_vertex;
attribute vec4 a_color;
attribute vec2 a_texcoord;

varying vec4 v_color;
varying vec2 v_texcoord;

void main()
{
	v_color = a_color;
	v_texcoord = a_texcoord;
	gl_Position = u_matViewProjection * u_matModel * vec4(a_vertex, 1.0);
}
