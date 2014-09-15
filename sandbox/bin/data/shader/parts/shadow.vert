struct Shadow {
    mat4 projectionMatrix;
    sampler2D map;
};

uniform Shadow shadows[4];
uniform uint numShadows;

out vec4 ps_shadow_position[4];

void applyShadow(mat4 matModel, vec3 position)
{
	ps_shadow_position[0] = shadows[0].projectionMatrix * matModel * vec4(position, 1.0);
	ps_shadow_position[1] = shadows[1].projectionMatrix * matModel * vec4(position, 1.0);
	ps_shadow_position[2] = shadows[2].projectionMatrix * matModel * vec4(position, 1.0);
	ps_shadow_position[3] = shadows[3].projectionMatrix * matModel * vec4(position, 1.0);
}

