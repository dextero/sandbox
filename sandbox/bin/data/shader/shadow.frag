#version 330

struct Shadow {
    mat4 projectionMatrix;
    sampler2DShadow map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

/*uniform vec3 eyePos;*/

uniform sampler2D tex;
uniform vec4 color;

const uint specularExp = 16u;
const float SHADOW_COEFFICIENT = 0.7;

in vec4 ps_shadow_position;
in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    float shadowCoefficient = 0.0f;
    for (uint i = 0u; i < numShadows; ++i) {
        shadowCoefficient += textureProj(shadows[0].map, ps_shadow_position);
    }
    shadowCoefficient /= float(numShadows);

    vec4 shadowColor = vec4(shadowCoefficient, shadowCoefficient, shadowCoefficient, 1.0);
    vec4 baseColor = vec4((color * texture2D(tex, ps_texcoord)).rgb, 1.0);

    out_color = (SHADOW_COEFFICIENT * shadowColor + 1.0 - SHADOW_COEFFICIENT) * baseColor;
}

