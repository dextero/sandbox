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

const uint specularExp = 16u;

uniform vec4 ambientLightColor;
uniform PointLight pointLights[8]; 
uniform uint numPointLights;
uniform ParallelLight parallelLights[8];
uniform uint numParallelLights;

#ifndef HAVE_EYEPOS
#define HAVE_EYEPOS
uniform vec3 eyePos;
#endif

vec4 diffuse(vec3 position,
             vec3 normal)
{
    vec4 outColor = vec4(0.0, 0.0, 0.0, 1.0);

    for (uint i = 0u; i < numParallelLights; ++i) {
        vec3 lightDir = normalize(parallelLights[i].direction);
        lightDir.y = -lightDir.y;
        outColor.rgb += parallelLights[i].color.rgb
                        * parallelLights[i].intensity
                        * dot(lightDir, normal);
    }

    for (uint i = 0u; i < numPointLights; ++i) {
        vec3 diff = pointLights[i].position - position;
        vec3 lightDir = normalize(diff);
        float distSquared = dot(diff, diff);
        outColor.rgb += pointLights[i].color.rgb
                        / distSquared
                        * pointLights[i].intensity
                        * dot(lightDir, normal);
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
        outColor.rgb += pointLights[i].color.rgb
                        * pointLights[i].intensity
                        * pow(dot(lightDir, reflect(eyeDir, normal)), specularExp);
    }

    return clamp(outColor, 0.0, 1.0);
}

vec4 applyLight(vec4 inColor,
                vec3 position,
                vec3 normal,
                float ambientCoefficient,
                float diffuseCoefficient,
                float specularCoefficient)
{
    normal = normalize(normal);

    vec4 ambientColor = ambientCoefficient * ambientLightColor;
    vec4 diffuseColor = diffuseCoefficient * diffuse(position, normal);
    vec4 specularColor = specularCoefficient * specular(position, normal);

    vec4 lightColor = ambientColor + diffuseColor + specularColor;

    return vec4(clamp(inColor.rgb * lightColor.rgb, 0.0, 1.0), inColor.a);
}

