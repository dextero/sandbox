#version 330

in vec3 in_position; // POSITION
in vec2 in_texcoord; // TEXCOORD

uniform mat4 matModel, matViewProjection;

out vec3 world_pos;
out vec2 texcoord;	

void main(){

	world_pos = (matModel * vec4(in_position,1)).xyz;
	texcoord = in_texcoord;
    gl_Position = matViewProjection * matModel * vec4(in_position,1); 
}
