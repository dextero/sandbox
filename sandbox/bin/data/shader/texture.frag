#version 330

uniform sampler2D texture;
uniform vec4 color;

in vec2 texcoord;

out vec4 out_color;

void main()
{
	out_color = color * texture2D(texture, texcoord);
}

