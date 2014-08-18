#version 330

/*
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Light lights[];
uniform uint num_lights;
uniform vec3 eye_pos;
*/

uniform sampler2D texture;
uniform vec4 color;

const uint specular_exp = 16u;

in vec4 ps_position;
in vec3 ps_normal;
in vec2 ps_texcoord;

out vec4 out_color;

/*
vec4 phong(vec3 position,
           vec3 normal,
           vec4 color)
{
    vec3 eye_dir = normalize(position - eye_pos);
    vec4 out_color = vec4(0.0, 0.0, 0.0, 1.0);

    for (uint i = 0u; i < num_lights; ++i) {
        vec3 light_dir = normalize(lights[i].position - position);

        float diffuse = dot(light_dir, normal);
        float specular = pow(dot(light_dir, reflect(eye_dir, normal)), specular_exp);

        out_color.xyz += lights[i].intensity * lights[i].color * (1.0 + diffuse + specular);
    }

    return out_color;
}
*/

void main()
{
    out_color = color * texture2D(texture, ps_texcoord);
}

