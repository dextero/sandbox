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
};

uniform vec4 ambientLightColor;
uniform PointLight pointLights[8]; 
uniform uint numPointLights;
uniform ParallelLight parallelLights[8];
uniform uint numParallelLights;

uniform vec3 eyePos;

uniform sampler2D tex;
uniform vec4 color;

const uint specularExp = 16u;

in vec4 ps_position;
in vec3 ps_normal;
in vec2 ps_texcoord;

out vec4 out_color;

vec4 diffuse(vec3 position,
             vec3 normal)
{
    vec4 outColor = vec4(0.0, 0.0, 0.0, 1.0);

    for (uint i = 0u; i < numParallelLights; ++i) {
        vec3 lightDir = parallelLights[i].direction;
        lightDir.y = -lightDir.y;
        outColor.rgb += parallelLights[i].color.rgb * parallelLights[i].intensity * dot(lightDir, normal);
    }

    for (uint i = 0u; i < numPointLights; ++i) {
        vec3 diff = position - pointLights[i].position;
        vec3 lightDir = normalize(diff);
        float distSquared = dot(diff, diff);
        outColor.rgb += pointLights[i].color.rgb * pointLights[i].intensity * dot(lightDir, normal);
    }

    return clamp(outColor, 0.0, 1.0);
}

vec4 specular(vec3 position,
              vec3 normal)
{
    vec4 outColor = vec4(0.0, 0.0, 0.0, 1.0);

    for (uint i = 0u; i < numPointLights; ++i) {
        vec3 lightDir = normalize(position - pointLights[i].position);
        vec3 eyeDir = normalize(eyePos - position);
        outColor.rgb += pointLights[i].color.rgb * pointLights[i].intensity * pow(dot(lightDir, reflect(eyeDir, normal)), specularExp);
    }

    return clamp(outColor, 0.0, 1.0);
}

vec4 phong(vec3 position,
           vec3 normal)
{
#if 1
    return clamp(ambientLightColor + diffuse(position, normal) + specular(position, normal), 0.0, 1.0);
#else
#  if 1
    return clamp(ambientLightColor + diffuse(position, normal), 0.0, 1.0);
#  else
    return clamp(ambientLightColor, 0.0, 1.0);
#  endif
#endif
}

void main()
{
    out_color = color * texture2D(tex, ps_texcoord) * phong(ps_position.xyz, ps_normal);
}

