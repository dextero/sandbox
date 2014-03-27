#version 330

uniform mat4 u_matViewProjection;
uniform mat4 u_matModel;

layout (location = 0) in vec3 a_vertex;
layout (location = 3) in vec4 a_color;

out vec4 v_color;

void main()
{
	v_color = a_color;
	gl_Position = u_matViewProjection * u_matModel * vec4(a_vertex, 1.0);
}

