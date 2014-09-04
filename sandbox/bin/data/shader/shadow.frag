#version 330

struct Shadow {
    mat4 projectionMatrix;
    sampler2DShadow map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

/*uniform vec3 eyePos;*/

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec4 color;

const uint specularExp = 16u;
const float SHADOW_COEFFICIENT = 0.7;

in vec4 ps_shadow_position;
in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    vec4 shadow_pos = ps_shadow_position;
    shadow_pos.z -= shadow_pos.w * 0.0001;

    float shadowCoefficient = 0.0;
    for (uint i = 0u; i < numShadows; ++i) {
        if (shadow_pos.x >= 0.0 && shadow_pos.x <= 1.0
                && shadow_pos.y >= 0.0 && shadow_pos.y <= 1.0) {
            shadowCoefficient += 1.0 - textureProj(shadows[0].map, shadow_pos);
        }
    }
    shadowCoefficient = 1.0 - shadowCoefficient / numShadows;

    vec4 shadowColor = vec4(shadowCoefficient, shadowCoefficient, shadowCoefficient, 1.0);
    vec4 baseColor = vec4((color * texture2D(tex, ps_texcoord)).rgb, 1.0);
    vec4 tex2Color = texture2D(tex2, ps_texcoord);

    out_color = (SHADOW_COEFFICIENT * shadowColor + 1.0 - SHADOW_COEFFICIENT) * (baseColor + tex2Color) / 2.0;
}

