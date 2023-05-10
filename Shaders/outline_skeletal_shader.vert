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

const int MAX_BONES = 256;
const int MAX_BONE_INFLUENCE = 4;

layout(set = 3, binding=0) uniform DynamicSkelUBO
{
    mat4[256] finalBonesMatrices;
}skel_dubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uvec4 inBoneIDs;
layout(location = 4) in vec4 inBoneWeights;

void main() {
    mat4 boneTransform = skel_dubo.finalBonesMatrices[inBoneIDs[0]] * inBoneWeights[0];
    boneTransform     += skel_dubo.finalBonesMatrices[inBoneIDs[1]] * inBoneWeights[1];
    boneTransform     += skel_dubo.finalBonesMatrices[inBoneIDs[2]] * inBoneWeights[2];
    boneTransform     += skel_dubo.finalBonesMatrices[inBoneIDs[3]] * inBoneWeights[3];

    gl_Position =  ubo.proj * ubo.view  * boneTransform * scaleMat(outline.scale) * vec4(inPosition, 1.0);
}