#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;

in vec3 position; // POSITION

void main()
{
    gl_Position = matViewProjection * matModel * vec4(position, 1.0);
}

