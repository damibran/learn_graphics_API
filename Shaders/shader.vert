#version 450

// viewport camera render data
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// per static model data
layout(set = 3, binding=0) uniform DynamicUBO
{
    mat4 model;
}dubo;

// model position of vertex
layout(location = 0) in vec3 inPosition;
// normal of vertex
layout(location = 1) in vec3 inNormal;
// UV texture coordinate
layout(location = 2) in vec2 inTexCoord;

// out texture coord for fragment shader
layout(location = 0) out vec2 fragTexCoord;
// out transformed normal
layout(location = 1) out vec3 outNormal;

void main() {
    // apply camera trasformations and model matrix
    gl_Position = ubo.proj * ubo.view * dubo.model * vec4(inPosition, 1.0);
    // pass texture coords
    fragTexCoord = inTexCoord;
    // transform normal with inverse transpose matrix
    // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    outNormal = normalize(mat3(transpose(inverse(dubo.model))) *inNormal);
}