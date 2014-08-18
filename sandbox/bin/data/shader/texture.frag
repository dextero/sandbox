#version 330

uniform sampler2D texture;
uniform vec4 color;

const uint specular_exp = 16u;

in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    out_color = color * texture2D(texture, ps_texcoord);
}

