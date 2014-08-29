#version 330

struct Shadow {
    mat4 projectionMatrix;
    sampler2DShadow map;
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
    vec3 shadowmapPos = ps_shadow_position.xyz / ps_shadow_position.w;
    float shadowCoefficient = texture(shadows[0].map, shadowmapPos);

    out_color = vec4(shadowCoefficient, shadowCoefficient, shadowCoefficient, 1.0) + 0.001 * vec4((color * texture2D(tex, ps_texcoord) * shadowCoefficient).rgb, 1.0);
}

