struct Shadow {
    mat4 projectionMatrix;
    sampler2D map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

in vec4 ps_shadow_position[4];

vec4 applyShadow(vec4 inColor)
{
    const float SHADOW_COEFFICIENT = 0.7;
    const float SELF_SHADOWING_FACTOR = 0.0001;
    const vec4 BLACK = vec4(0.0, 0.0, 0.0, 1.0);

    float shadowCoefficient = 0.0;

    vec4 shadowPosition = ps_shadow_position[0];
    shadowPosition.xyz /= shadowPosition.w;
    shadowPosition.z -= SELF_SHADOWING_FACTOR;

    if (shadowPosition.x >= 0.0 && shadowPosition.x <= 1.0 && shadowPosition.y >= 0.0 && shadowPosition.y <= 1.0) {
        float depth = texture2D(shadows[0].map, shadowPosition.xy).r;
        float shadowFactor = (depth < shadowPosition.z) ? 1.0 : 0.0;
        shadowCoefficient += clamp(shadowFactor, 0.0, 1.0);
    }

    shadowPosition = ps_shadow_position[1];
    shadowPosition.xyz /= shadowPosition.w;
    shadowPosition.z -= SELF_SHADOWING_FACTOR;

    if (shadowPosition.x >= 0.0 && shadowPosition.x <= 1.0 && shadowPosition.y >= 0.0 && shadowPosition.y <= 1.0) {
        float depth = texture2D(shadows[1].map, shadowPosition.xy).r;
        float shadowFactor = (depth < shadowPosition.z) ? 1.0 : 0.0;
        shadowCoefficient += clamp(shadowFactor, 0.0, 1.0);
    }

    shadowCoefficient = 1.0 - shadowCoefficient / numShadows;

    return mix(BLACK, inColor, shadowCoefficient);
}

