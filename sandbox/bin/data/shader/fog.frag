#version 330

#include "data/shader/parts/fog.frag"

uniform sampler2D tex;

uniform vec4 color;

in vec3 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    const vec3 FOG_COLOR = vec3(0.4, 0.4, 0.45);
    const float FOG_DENSITY = 0.01;

    out_color = texture2D(tex, ps_texcoord) * color;
    out_color = applyFog(out_color, ps_position, FOG_COLOR, FOG_DENSITY);
}

