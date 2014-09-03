#version 330

struct Shadow {
    mat4 projectionMatrix;
    sampler2D map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

/*uniform vec3 eyePos;*/

uniform sampler2D tex;
uniform vec4 color;

const uint specularExp = 16u;

in vec4 ps_shadow_position;
in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    vec4 proj = ps_shadow_position / ps_shadow_position.w;

    float shadowCoefficient = 0.0f;
    for (uint i = 0u; i < numShadows; ++i) {
        float depth = texture2D(shadows[i].map, ps_shadow_position.st).r;
        shadowCoefficient += depth < ps_shadow_position.z ? 0.0 : 1.0;
        /*shadowCoefficient += clamp(abs(textureProj(shadows[0].map, shpos)), 0.0, 1.0);*/
    }
    shadowCoefficient /= float(numShadows);

    vec4 shadowColor = vec4(shadowCoefficient, shadowCoefficient, shadowCoefficient, 1.0);
    vec4 baseColor = vec4((color * texture2D(tex, ps_texcoord)).rgb, 1.0);

    out_color = shadowColor * baseColor;
}

