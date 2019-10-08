precision highp float;

in vec2 v_tex_coord;

void main()
{
    outputFragmentColor = vec4(v_tex_coord.x, v_tex_coord.y, 0.0, 1.0);
}

