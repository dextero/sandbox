#version 330

uniform mat4 matViewProjection;
uniform mat4 matModel;
/**/uniform mat3 matNormal;

in vec3 position; // POSITION
in vec2 texcoord; // TEXCOORD
in vec3 normal; // NORMAL
/*in vec3 tangent; // TANGENT*/
/*in vec3 bitangent; // BITANGENT*/

out vec4 ps_position;
out vec2 ps_texcoord;
/*out vec3 ps_normal;*/

/*out mat3 ps_tangent_to_world;*/

void main()
{
    ps_position = matModel * vec4(position, 1.0);
    ps_texcoord = texcoord;
    /*ps_normal = normalize(matNormal * normal);*/

    /*ps_tangent_to_world = mat3(*/
        /*normalize(tangent),*/
        /*normalize(normal),*/
        /*normalize(bitangent)*/
    /*);*/

    gl_Position = matViewProjection * ps_position;
}

