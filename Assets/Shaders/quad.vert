precision highp float;

// xy = position
// zw = texture coords
in vec4 a_vertex;

out vec2 v_tex_coord;

uniform mat4 u_model;
uniform mat4 u_projection;

void main()
{
    v_tex_coord = a_vertex.zw;

    gl_Position = u_projection * u_model * vec4(a_vertex.xy, 0.0, 1.0);
}
