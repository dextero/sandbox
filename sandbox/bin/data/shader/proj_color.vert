#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position;
in vec4 color;

out vec4 out_color;

void main()
{
	out_color = color;
	gl_Position = matViewProjection * matModel * vec4(position, 1.0);
}

