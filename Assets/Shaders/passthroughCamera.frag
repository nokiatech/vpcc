precision mediump float;

in vec2 v_tex_coord;

uniform samplerExternalOES u_texture;

void main()
{
    outputFragmentColor = vec4(texture(u_texture, v_tex_coord).rgb, 1.0);
}
