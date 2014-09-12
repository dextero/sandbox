#version 330

#include "data/shader/parts/light.frag"

uniform sampler2D tex;
uniform vec4 color;

in vec3 ps_position;
in vec3 ps_normal;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    out_color = texture2D(tex, ps_texcoord) * color;
    out_color = applyLight(out_color, ps_position, ps_normal, 0.001, 1.0, 0.001);
}

