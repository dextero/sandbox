#version 330

uniform sampler2D tex;

uniform vec4 color;
uniform vec3 eyePos;

in vec3 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

vec4 applyFog(vec4 inColor,
              vec3 worldPos,
              vec3 fogColor,
              float fogDensity)
{
    float dist = distance(eyePos, worldPos);
    float exponent = dist * fogDensity;
    float fogFactor = 1.0 / exp(exponent * exponent);
    fogFactor = clamp(fogFactor, 0.0, 1.0); 

    return vec4(inColor.rgb * fogFactor, 1.0);
}

void main()
{
    const vec3 FOG_COLOR = vec3(0.0, 0.0, 1.0);
    const float FOG_DENSITY = 0.05;

    out_color = applyFog(texture2D(tex, ps_texcoord) * color,
                         ps_position, FOG_COLOR, FOG_DENSITY);
}

