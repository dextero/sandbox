#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position; // POSITION
in vec2 texcoord; // TEXCOORD

out vec4 ps_position;
out vec2 ps_texcoord;

void main()
{
    vec4 worldPos = matModel * vec4(position, 1.0);

    ps_position = worldPos;
    ps_texcoord = texcoord;

    gl_Position = matViewProjection * worldPos;
}

