#version 330

#include "data/shader/parts/fog.frag"
#include "data/shader/parts/light.frag"
#include "data/shader/parts/shadow.frag"

uniform sampler2D tex;

uniform vec4 color;

in vec3 ps_position;
in vec2 ps_texcoord;
in vec3 ps_normal;

out vec4 out_color;

void main()
{
    const float FOG_DENSITY = 0.002;

    out_color = texture2D(tex, ps_texcoord) * color;
    out_color = applyLight(out_color, ps_position, ps_normal, 0.001, 1.0, 0.001);
    out_color = applyShadow(out_color);
    out_color = applyFog(out_color, ps_position, FOG_DENSITY);
}

