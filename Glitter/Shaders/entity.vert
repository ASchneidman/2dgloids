#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in mat4 instanceMatrix;
layout (location = 6) in vec3 color;

uniform mat4 model;
uniform mat4 projection;

out vec3 inColor;

void main()
{
    //gl_Position = projection * model * vec4(aPos.xy, 0.0f, 1.0f);
    gl_Position = projection * instanceMatrix * vec4(aPos.xy, 0.0f, 1.0f);
    inColor = color;
}