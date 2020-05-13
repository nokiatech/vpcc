#ifdef GL_ES
precision highp float;
#endif

in vec2 v_tex_coord;

uniform samplerVideo u_texture0;
uniform vec3 u_tint_color;

void main()
{
    outputFragmentColor = vec4(u_tint_color, 1.0) * texture(u_texture0, v_tex_coord);
}
