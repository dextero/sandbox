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
    for (uint i = 0u; i < numShadows; ++i) {
        vec4 shadowPosition = ps_shadow_position[i];
        shadowPosition.xyz /= shadowPosition.w;
        shadowPosition.z -= SELF_SHADOWING_FACTOR;

        if (shadowPosition.x >= 0.0 && shadowPosition.x <= 1.0
                && shadowPosition.y >= 0.0 && shadowPosition.y <= 1.0) {
            float depth = texture2D(shadows[i].map, shadowPosition.xy).r;
            float depthDiff = exp(100.0 * (depth - shadowPosition.z));
            float shadowFactor = (depth < shadowPosition.z) ? depthDiff : 0.0;
            shadowCoefficient += clamp(shadowFactor, 0.0, 1.0);
        }
    }
    shadowCoefficient = clamp((1.0 - shadowCoefficient / numShadows)/* + 0.5*/, 0.0, 1.0);

    return mix(BLACK, inColor, shadowCoefficient);
}

