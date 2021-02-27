#version 330 core
in vec4 v_position;
in vec3 v_color;
out vec4 FragColor;
void main()
{
    FragColor = vec4(v_color, 1.0);
    /*
    if (v_position.z >= 0.0)
    {
      float light_green = 0.5 + v_position.z / 2.0;
      FragColor = vec4(0.0, light_green, 0.0, 1.0);
    } else
    {
      float dark_green = 0.5 - (v_position.z / -2.0);
      FragColor = vec4(0.0, dark_green, 0.0, 1.0);
    }
    */
}