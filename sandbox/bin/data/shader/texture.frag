#version 330

uniform sampler2D texture;
uniform vec4 color;

in vec4 in_color;
in vec2 texcoord;

out vec4 out_color;

void main()
{
	out_color = color * in_color * texture2D(texture, texcoord);
}

