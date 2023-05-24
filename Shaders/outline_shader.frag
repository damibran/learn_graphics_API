#version 450

// outline shader data
layout(set = 1, binding=0) uniform OutlineData{
    // outline color
    vec3 color;
    // outline scale
    float scale;
} outline;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(outline.color, 1.0);
}