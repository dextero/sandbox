#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position; // POSITION
in vec3 normal;   // NORMAL
in vec4 color;    // COLOR

out vec4 ps_position;
out vec3 ps_normal;
out vec4 ps_color;

void main()
{
    ps_position = matViewProjection * matModel * vec4(in_position, 1.0);
    ps_normal = normalize(mat3(matModel) * in_normal);
    ps_color = in_color;

    gl_Position = ps_position;
}

