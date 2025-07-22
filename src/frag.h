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

const char* yuv_fragment_shader = R"(
precision mediump float;

// Samplers for the three separate Y, U, and V texture planes
uniform sampler2D u_texture_y;
uniform sampler2D u_texture_u;
uniform sampler2D u_texture_v;

varying vec2 v_texCoord;

void main()
{
    // Flip the UV coordinates vertically to match OpenGL's texture coordinate system
    vec2 flipped_uv = vec2(v_texCoord.x, 1.0 - v_texCoord.y);

    // Sample the luma (brightness) and chroma (color) values from their respective textures.
    // The same texture coordinates are used for all three; the GPU handles scaling
    // correctly if the U and V planes are a lower resolution than the Y plane.
    float y = texture2D(u_texture_y, flipped_uv).r;
    float u = texture2D(u_texture_u, flipped_uv).r;
    float v = texture2D(u_texture_v, flipped_uv).r;

    // The U and V values are centered around 0.5, so they must be shifted
    // back to the [-0.5, 0.5] range for the conversion math.
    u = u - 0.5;
    v = v - 0.5;

    // Standard YUV to RGB conversion formula
    float r = y + (1.402 * v);
    float g = y - (0.344136 * u) - (0.714136 * v);
    float b = y + (1.772 * u);

    // Set the final color, ensuring the values are clamped to the [0.0, 1.0] range,
    // with a full alpha channel.
    gl_FragColor = vec4(clamp(vec3(r, g, b), 0.0, 1.0), 1.0);
}
)";
