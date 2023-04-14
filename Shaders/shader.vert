#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 3, binding=0) uniform DynamicUBO
{
    mat4 model;
}dubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outNormal;

void main() {
    gl_Position = ubo.proj * ubo.view * dubo.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    outNormal = mat3(transpose(inverse(dubo.model))) *inNormal;
}