#version 450

layout(location = 0) out vec4 outColor;

layout(set = 2, binding=0) uniform OutlineData{
    vec3 color;
    float scale;
} outline;

void main() {
    outColor = vec4(outline.color, 1.0);
}