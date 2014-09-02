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

in vec4 ps_shadow_position;
in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    float shadowCoefficient = clamp(float(numShadows), 0.99, 1.01);
    shadowCoefficient *= clamp(textureProj(shadows[0].map, ps_shadow_position), 0.0, 1.0);

    vec4 shadowColor = vec4(shadowCoefficient, shadowCoefficient, shadowCoefficient, 1.0);
    vec4 baseColor = vec4((color * texture2D(tex, ps_shadow_position.xy)).rgb, 1.0);

    out_color = (0.5 * shadowColor + 0.5) * baseColor;
}

