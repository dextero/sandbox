#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 in_position; // POSITION
in vec3 in_normal;   // NORMAL
in vec4 in_color;    // COLOR

out vec3 position;
out vec3 normal;
out vec4 color;

void main()
{
	position = matViewProjection * matModel * vec4(in_position, 1.0);
    normal = in_normal;
	color = in_color;

    gl_Position = position;
}

