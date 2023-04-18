#version 450
//#extension GL_KHR_glsl : enable

const vec3 light_dir = vec3(0,0,1);

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
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    
    const vec4 lightColor = vec4(1.0,1.0,1.0,1.0);

    const float ambientStrength = 1.0;
    vec4 ambient = ambientStrength * lightColor;

    float diff = 0.05*max(dot(inNormal,light_dir),0.0);
    vec4 diffuse = diff * lightColor;

    outColor = (ambient + diffuse)*0.5*properties.base_color * texture(texSampler, fragTexCoord);
}