#ifndef HAVE_EYEPOS
#define HAVE_EYEPOS
uniform vec3 eyePos;
#endif

vec4 applyFog(vec4 inColor,
              vec3 worldPos,
              vec3 fogColor,
              float fogDensity)
{
    float dist = distance(eyePos, worldPos);
    float exponent = dist * fogDensity;
    float fogFactor = 1.0 / exp(exponent * exponent);
    fogFactor = clamp(fogFactor, 0.0, 1.0); 

    return vec4(inColor.rgb, fogFactor);
}

