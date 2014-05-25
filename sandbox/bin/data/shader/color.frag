#version 330

uniform vec4 color;

in vec4 in_color;

out vec4 out_color;

void main()
{
	out_color = color * in_color;
}

