#version 450
#extension GL_KHR_glsl : enable

layout(set=1,binding=0) uniform UnLitTexturedUBO
{
float gamma;
}ult;

layout(set=2, binding = 0) uniform sampler2D texSampler;

layout(set=2, binding = 1) uniform Properties
{
    vec4 base_color;
}properties;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = properties.base_color * texture(texSampler, fragTexCoord);
}