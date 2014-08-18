#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position; // POSITION
in vec2 texcoord; // TEXCOORD

out vec2 out_texcoord;

void main()
{
	out_texcoord = texcoord;
	gl_Position = matViewProjection * matModel * vec4(position, 1.0);
}

