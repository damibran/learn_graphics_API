#version 450
// viewport camera render data
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// skeletal constants MUST BE IN SYNC with same in dmbrn::BonedVertex
const int MAX_BONES = 256;
const int MAX_BONE_INFLUENCE = 4;

// per skeletal model data
layout(set = 3, binding=0) uniform DynamicSkelUBO
{
    mat4[256] finalBonesMatrices;
}skel_dubo;


// model position of vertex
layout(location = 0) in vec3 inPosition;
// normal of vertex
layout(location = 1) in vec3 inNormal;
// UV texture coordinate
layout(location = 2) in vec2 inTexCoord;
// bone indexs of bones influence this from finalBonesMatrices
layout(location = 3) in uvec4 inBoneIDs;
// weights of influences bones
layout(location = 4) in vec4 inBoneWeights;

// out texture coord for fragment shader
layout(location = 0) out vec2 fragTexCoord;
// out transformed normal
layout(location = 1) out vec3 outNormal;

void main() 
{
    // accumalate all influences bone transformaton with weights
    mat4 boneTransform = skel_dubo.finalBonesMatrices[inBoneIDs[0]] * inBoneWeights[0];
    boneTransform     += skel_dubo.finalBonesMatrices[inBoneIDs[1]] * inBoneWeights[1];
    boneTransform     += skel_dubo.finalBonesMatrices[inBoneIDs[2]] * inBoneWeights[2];
    boneTransform     += skel_dubo.finalBonesMatrices[inBoneIDs[3]] * inBoneWeights[3];	

    // apply camera trasformations and model matrix
    gl_Position = ubo.proj * ubo.view  * boneTransform * vec4(inPosition, 1.0);
    // pass texture coords to fragment shader
    fragTexCoord = inTexCoord;
    // transform normal with inverse transpose matrix
    // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    outNormal = normalize(mat3(transpose(inverse(boneTransform)))*inNormal);
}