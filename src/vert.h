#pragma once

const char* vertex_shader = R"(
uniform mat4 u_Transform;
uniform mat4 u_Projection;

attribute vec4 a_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

void main()
{
   gl_Position = u_Projection * u_Transform * a_position;
   v_texCoord = a_texCoord;
}
)";
