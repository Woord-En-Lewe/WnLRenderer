#pragma once

const char* fragment_shader = R"(
precision mediump float;

uniform sampler2D u_texture;
varying vec2 v_texCoord;

void main()
{
    vec2 flipped_uv = vec2(v_texCoord.x, 1.0 - v_texCoord.y);
    gl_FragColor    = texture2D(u_texture, flipped_uv);
}
)";
