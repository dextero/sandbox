#version 330

uniform mat4 matModelViewProjection;

in vec3 position; // POSITION

void main()
{
    gl_Position = matModelViewProjection * vec4(position, 1.0);
}

