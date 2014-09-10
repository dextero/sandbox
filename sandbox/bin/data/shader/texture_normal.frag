#version 330

#include "data/shader/parts/light.frag"

uniform sampler2D tex;
uniform vec4 color;

in vec4 ps_position;
in vec3 ps_normal;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    out_color = applyLight(color * texture2D(tex, ps_texcoord),
                           ps_position.xyz, ps_normal, 1.0, 1.0, 1.0);
}

