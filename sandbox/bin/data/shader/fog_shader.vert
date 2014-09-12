#version 330

#include "data/shader/parts/shadow.vert"

in vec3 in_position; // POSITION
in vec2 in_texcoord; // TEXCOORD
in vec3 in_normal; // NORMAL

uniform mat4 matModel;
uniform mat4 matViewProjection;
uniform mat3 matNormal;

out vec3 ps_position;
out vec2 ps_texcoord;
out vec3 ps_normal;

void main()
{
    applyShadow(matModel, ps_position);

    vec4 worldPos = matModel * vec4(in_position, 1.0);

    ps_position = worldPos.xyz;
    ps_texcoord = in_texcoord;
    ps_normal = normalize(matNormal * in_normal);

    gl_Position = matViewProjection * worldPos;
}

