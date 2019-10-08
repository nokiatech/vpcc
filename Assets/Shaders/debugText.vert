precision highp float;

in vec2 a_position;
in vec2 a_tex_coord;
in vec4 a_text_color;
in vec4 a_background_color;

out vec2 v_tex_coord;
out vec4 v_text_color;
out vec4 v_background_color;

uniform mat4 u_projection;

void main()
{
    v_tex_coord = a_tex_coord;
    v_text_color = a_text_color;
    v_background_color = a_background_color;

    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
}
