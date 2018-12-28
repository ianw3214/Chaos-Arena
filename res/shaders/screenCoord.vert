#version 400 core

layout(location = 0) in ivec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;

uniform int u_screen_width;
uniform int u_screen_height;

void main() {
    float x = mix(-1.0f, 1.0f, float(position.x) / float(u_screen_width));
    float y = mix(-1.0f, 1.0f, float(position.y) / float(u_screen_height));

    // Invert y so that the origin is on the top left corner
    gl_Position = vec4(x, -y, 0.0f, 1.0f);

    // Pass on the texture coordinate
    vTexCoord = texCoord;
}