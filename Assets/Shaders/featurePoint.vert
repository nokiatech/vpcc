in vec4 a_position;

uniform mat4 u_mvp;
uniform vec4 u_color;
uniform float u_point_size;

out vec4 v_color;

void main()
{
   v_color = u_color;

   gl_Position = u_mvp * vec4(a_position.xyz, 1.0);
   gl_PointSize = u_point_size;
}
