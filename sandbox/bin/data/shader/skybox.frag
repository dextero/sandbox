#version 330

uniform sampler2D tex;
uniform vec4 color;
uniform vec3 sunPos;

in vec4 ps_position;
in vec2 ps_texcoord;

out vec4 out_color;

void main()
{
    vec3 sunPosNormalized = normalize(sunPos);
    vec3 posNormalized = normalize(ps_position.xyz);

    float dirDot = dot(sunPosNormalized, posNormalized);
    dirDot *= exp(-sunPosNormalized.y);
    dirDot = clamp(dirDot, 0.0, 1.0);
    dirDot *= exp(-posNormalized.y);
    dirDot = clamp(dirDot, 0.3, 1.0);
    dirDot *= 1.0 - abs(sunPosNormalized.y * 2.0);
    dirDot = clamp(dirDot, 0.0, 1.0);
    dirDot /= 2.0;

    vec4 dawnColor = vec4(1.0 + dirDot, 1.0 - dirDot, 1.0 - 0.7 * dirDot, 1.0);
    out_color = dawnColor * color * texture2D(tex, ps_texcoord);
}

