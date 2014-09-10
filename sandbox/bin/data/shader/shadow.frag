#version 330

#include "data/shader/parts/shadow.frag"

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec4 color;

in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    vec4 baseColor = vec4((color * texture2D(tex, ps_texcoord)).rgb, 1.0);
    vec4 tex2Color = texture2D(tex2, ps_texcoord);

    out_color = applyShadow((baseColor + tex2Color) / 2.0);
}

