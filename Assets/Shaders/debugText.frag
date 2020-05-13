#ifdef GL_ES
precision highp float;
#endif

in vec2 v_tex_coord;
in vec4 v_text_color;
in vec4 v_background_color;

uniform sampler2D u_texture0;

void main()
{
    outputFragmentColor = mix(v_background_color, v_text_color, texture(u_texture0, v_tex_coord).r);
}
