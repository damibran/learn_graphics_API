#version 450

#define scaleMat(scale) mat4(vec4(scale,0,0,0),vec4(0,scale,0,0),vec4(0,0,scale,0),vec4(0,0,0,1))

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding=0) uniform OutlineData{
    vec3 color;
    float scale;
} outline;

layout(set = 3, binding=0) uniform DynamicUBO
{
    mat4 model;
}dubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

void main() {
    gl_Position =  ubo.proj * ubo.view  * dubo.model * vec4(inPosition + outline.scale * inNormal, 1.0);
}