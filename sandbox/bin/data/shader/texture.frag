uniform sampler2D u_texture;
uniform vec4 u_color;

varying vec4 v_color;
varying vec2 v_texcoord;

void main()
{
	gl_FragColor = u_color * v_color * texture2D(u_texture, v_texcoord);
}
