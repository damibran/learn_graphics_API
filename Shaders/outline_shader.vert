#version 450

#define outline 1.05
#define scaleMat mat4(vec4(outline,0,0,0),vec4(0,outline,0,0),vec4(0,0,outline,0),vec4(0,0,0,1))

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 2, binding=0) uniform DynamicUBO
{
    mat4 model;
}dubo;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = ubo.proj * ubo.view *  dubo.model *scaleMat* vec4(inPosition, 1.0);
}