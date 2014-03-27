#version 330

uniform sampler2D u_texture;
uniform vec4 u_color;

in vec4 v_color;
in vec2 v_texcoord;

out vec4 out_color;

void main()
{
	out_color = u_color * v_color * texture2D(u_texture, v_texcoord);
}

