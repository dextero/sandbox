#version 330

uniform mat4 u_matViewProjection;
uniform mat4 u_matModel;

layout (location = 0) in vec3 a_vertex;
layout (location = 3) in vec4 a_color;
layout (location = 7) in vec2 a_texcoord;

out vec4 v_color;
out vec2 v_texcoord;

void main()
{
	v_color = a_color;
	v_texcoord = a_texcoord;
	gl_Position = u_matViewProjection * u_matModel * vec4(a_vertex, 1.0);
}

