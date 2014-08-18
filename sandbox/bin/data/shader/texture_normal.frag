#version 330

struct PointLight {
    vec3 position;
    vec4 color;
    float intensity;
};
struct ParallelLight {
    vec3 direction;
    vec4 color;
    float intensity;
}

uniform vec3 ambientLightColor;
uniform PointLight pointLights[]; 
uniform uint numPointLights;
uniform ParallelLight parallelLights[];
uniform uint numParallelLights;

uniform vec3 eyePos;

uniform sampler2D texture;
uniform vec4 color;

const uint specular_exp = 16u;

in vec4 ps_position;
in vec3 ps_normal;
in vec2 ps_texcoord;

out vec4 out_color;

vec4 phong(vec3 position,
           vec3 normal)
{
    vec3 eye_dir = normalize(position - eye_pos);
    vec4 out_color = vec4(0.0, 0.0, 0.0, 1.0);

    for (uint i = 0u; i < numParallelLights; ++i) {
        vec3 light_dir = -parallelLights[i].direction;

        float diffuse = dot(light_dir, normal);
        float specular = pow(dot(light_dir, reflect(eye_dir, normal)), specular_exp);

        out_color.xyz += parallelLights[i].intensity * parallelLights[i].color * (1.0 + diffuse + specular);
    }

    for (uint i = 0u; i < numPointLights; ++i) {
        vec3 light_dir = normalize(pointLights[i].position - position);

        float diffuse = dot(light_dir, normal);
        float specular = pow(dot(light_dir, reflect(eye_dir, normal)), specular_exp);

        out_color.xyz += pointLights[i].intensity * pointLights[i].color * (1.0 + diffuse + specular);
    }

    return out_color;
}

void main()
{
    out_color = color * texture2D(texture, ps_texcoord) * phong(ps_position.xyz, ps_normal);
}

