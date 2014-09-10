struct Shadow {
    mat4 projectionMatrix;
    sampler2DShadow map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

in vec4 ps_shadow_position[4];

vec4 applyShadow(vec4 inColor)
{
    const float SHADOW_COEFFICIENT = 0.7;
    const vec4 BLACK = vec4(0.0, 0.0, 0.0, 1.0);

    float shadowCoefficient = 0.0;
    for (uint i = 0u; i < numShadows; ++i) {
        vec4 shadowPosition = ps_shadow_position[i];
        shadowPosition.z -= shadowPosition.w * 0.0001;

        for (uint i = 0u; i < numShadows; ++i) {
            if (shadowPosition.x >= 0.0 && shadowPosition.x <= 1.0
                    && shadowPosition.y >= 0.0 && shadowPosition.y <= 1.0) {
                shadowCoefficient += 1.0 - textureProj(shadows[0].map, shadowPosition);
            }
        }
    }
    shadowCoefficient = 1.0 - shadowCoefficient / numShadows;

    return mix(BLACK, inColor, shadowCoefficient);
}

