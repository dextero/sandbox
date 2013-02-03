#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out varying vec4 v_color;

void main(void)
{
	for (int i = 0; i < gl_in.length(); ++i)
	{
		gl_Position = vec4(gl_in[i].gl_Position + vec3(-0.1, -0.1, 0.0), 1.0);
		v_color = vec4(1.0, 0.0, 0.0, 1.0);//v_texcoord = vec2(0.0, 0.0);
		EmitVertex();
		gl_Position = vec4(gl_in[i].gl_Position + vec3(0.1, -0.1, 0.0), 1.0);
		v_color = vec4(0.0, 1.0, 0.0, 1.0);//v_texcoord = vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = vec4(gl_in[i].gl_Position + vec3(-0.1, 0.5, 0.0), 1.0);
		v_color = vec4(0.0, 0.0, 1.0, 1.0);//v_texcoord = vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = vec4(gl_in[i].gl_Position + vec3(0.1, 0.1, 0.0), 1.0);
		v_color = vec4(1.0, 1.0, 0.0, 1.0);//v_texcoord = vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
}