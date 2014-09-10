#version 330

in vec3 in_position; // POSITION
in vec2 in_texcoord; // TEXCOORD

uniform mat4 matModel, matViewProjection;

out vec3 ps_position;
out vec2 ps_texcoord;	

void main()
{
    vec4 worldPos = matModel * vec4(in_position, 1.0);

	ps_position = worldPos.xyz;
	ps_texcoord = in_texcoord;

    gl_Position = matViewProjection * worldPos;
}

