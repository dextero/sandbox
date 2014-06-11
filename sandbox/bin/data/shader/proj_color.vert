#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 in_position;
in vec3 in_normal;
in vec4 in_color;

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

