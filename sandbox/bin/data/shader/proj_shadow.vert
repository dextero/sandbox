#version 330

#include "data/shader/parts/shadow.vert"

uniform mat4 matViewProjection;
uniform mat4 matModel;
uniform mat3 matNormal;

in vec3 position; // POSITION
in vec2 texcoord; // TEXCOORD
in vec3 normal; // NORMAL

out vec3 ps_position;
out vec2 ps_texcoord;
out vec3 ps_normal;

void main()
{
    applyShadow(matModel, position);

    vec4 worldPos = matModel * vec4(position, 1.0);

    ps_position = worldPos.xyz;
    ps_texcoord = texcoord;
    ps_normal = matNormal * normal;

    gl_Position = matViewProjection * worldPos;
}

