#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position;
in vec4 color;
in vec2 texcoord;

out vec4 out_color;
out vec2 out_texcoord;

void main()
{
	out_color = color;
	out_texcoord = texcoord;
	gl_Position = matViewProjection * matModel * vec4(position, 1.0);
}

