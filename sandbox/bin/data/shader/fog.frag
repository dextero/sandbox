#version 330

out vec4 out_color;

uniform vec3 light_position;
uniform vec3 eye_position;

uniform sampler2D texture1;

uniform vec4 color;
uniform vec3 eyePos;

in vec3 world_pos;
in vec2 texcoord;



vec4 generatefogColor(){
	
	const vec3 world_normal = vec3(0.0, 1., 0.);
	const vec3 DiffuseLight = vec3(0.15, 0.05, 0.0);
	const vec3 RimColor  = vec3(0.2, 0.2, 0.2);
	const vec3 fogColor = vec3(0.0, 0.0, 1.);
	const float FogDensity = 0.05;

    vec3 tex1 = texture(texture1, texcoord).rgb;

	vec3 L = normalize( light_position - world_pos);
	vec3 V = normalize( eye_position - world_pos);

	vec3 diffuse = DiffuseLight * max(0, dot(L,world_normal));
	
	float rim = 1 - max(dot(V, world_normal), 0.0);
	rim = smoothstep(0.6, 1.0, rim);
	vec3 finalRim = RimColor * vec3(rim, rim, rim);

	vec3 lightColor = finalRim + diffuse + tex1;

	float dist = distance(eyePos, world_pos);
	float fogFactor = 1.0 /exp( (dist * FogDensity)*(dist * FogDensity));
	fogFactor = clamp( fogFactor, 0.0, 1.0 ); 

	vec3 finalColor = fogColor*fogFactor;
	
    return vec4(finalColor, 1) + 0.1 * color;
}

void main(){

	out_color = generatefogColor();
}