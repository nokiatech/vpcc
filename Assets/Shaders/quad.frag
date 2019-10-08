precision highp float;

in vec2 v_tex_coord;

uniform sampler2D u_texture0;
uniform vec4 u_tint_color;

void main()
{
    outputFragmentColor = u_tint_color * texture(u_texture0, v_tex_coord);
}
