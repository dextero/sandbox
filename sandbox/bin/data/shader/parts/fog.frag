#ifndef HAVE_EYEPOS
#define HAVE_EYEPOS
uniform vec3 eyePos;
#endif

vec4 applyFog(vec4 inColor,
              vec3 worldPos,
              float fogDensity)
{
    float dist = distance(eyePos.xz, worldPos.xz);
    float exponent = dist * fogDensity;
    exponent *= exponent;
    float fogFactor = 1.0 / exp(exponent * exponent);
    fogFactor = clamp(fogFactor, 0.0, 1.0); 

    return vec4(inColor.rgb, fogFactor);
}

