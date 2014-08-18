#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position; // POSITION
in vec3 normal;   // NORMAL
in vec2 texcoord; // TEXCOORD

out vec4 ps_position;
out vec3 ps_normal;
out vec2 ps_texcoord;

void main()
{
    ps_position = matViewProjection * matModel * vec4(position, 1.0);
    ps_normal = normalize(mat3(matModel) * normal);
    ps_texcoord = texcoord;

    gl_Position = ps_position;
}

