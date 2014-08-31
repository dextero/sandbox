#version 330

#define SHADOW 1

struct Shadow {
    mat4 projectionMatrix;
#if SHADOW
    sampler2DShadow map;
#else
    sampler2D map;
#endif
};

uniform Shadow shadows[4];
uniform uint numShadows;

uniform vec3 eyePos;

uniform sampler2D tex;
uniform vec4 color;

const uint specularExp = 16u;

in vec4 ps_shadow_position;
in vec4 ps_position;
in vec3 ps_normal;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
#if SHADOW
    /*vec3 shadowmapPos = ps_shadow_position.xyz / ps_shadow_position.w;*/
    vec3 shadowmapPos = vec3(ps_texcoord, 0.0);
    float shadowCoefficient = texture(shadows[0].map, shadowmapPos);

    out_color = vec4(shadowCoefficient, shadowCoefficient, shadowCoefficient, 1.0) + 0.001 * vec4((color * texture2D(tex, ps_texcoord) * shadowCoefficient).rgb, 1.0);
#else
    vec3 pos = ps_shadow_position.xyz / ps_shadow_position.w;
    vec4 col = texture2D(shadows[0].map, pos.xy);
    out_color = col + 0.001 * vec4((color * texture2D(tex, ps_texcoord)).rgb, 1.0);
#endif
}

