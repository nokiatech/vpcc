#ifdef GL_ES
precision highp float;
#endif

in vec4 a_position;
in vec2 a_tex_coord;

out vec2 v_tex_coord;

uniform mat4 u_model;
uniform mat4 u_projection;

void main()
{
    v_tex_coord = a_tex_coord;

    gl_Position = u_projection * u_model * a_position;
}
