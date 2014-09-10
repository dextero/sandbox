struct Shadow {
    mat4 projectionMatrix;
    sampler2DShadow map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

out vec4 ps_shadow_position[4];

void applyShadow(mat4 matModel,
                 vec3 position)
{
    for (uint i = 0u; i < numShadows; ++i) {
        ps_shadow_position[i] = shadows[i].projectionMatrix
                                * matModel
                                * vec4(position, 1.0);
    }
}

