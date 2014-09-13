#version 330

#include "data/shader/parts/light.frag"

uniform sampler2D tex;
/**/uniform sampler2D bumpmap;
uniform vec4 color;
/**/uniform mat3 matNormal;

in vec4 ps_position;
in vec2 ps_texcoord;
/*in vec3 ps_normal;*/

/*in mat3 ps_tangent_to_world;*/

out vec4 out_color;

void main()
{
    vec3 normal = texture2D(bumpmap, ps_texcoord).rgb;// * 2.0 - 1.0;
    /*normal = normalize(ps_tangent_to_world * (normal));*/

    out_color = texture2D(tex, ps_texcoord) * color;
    out_color = applyLight(out_color, ps_position.xyz, normal, 0.001, 1.0, 0.001);

    /*out_color = color * vec4(normal, 1.0);*/
    out_color = 0.001 * out_color + vec4(normal, 1.0);
}

