#version 450

// construt sacle matrix with given scale
#define scaleMat(scale) mat4(vec4(scale,0,0,0),vec4(0,scale,0,0),vec4(0,0,scale,0),vec4(0,0,0,1))

// viewport camera render data
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// outline shader effect data
layout(set = 1, binding=0) uniform OutlineData{
    vec3 color;
    float scale;
} outline;

// per static object data
layout(set = 3, binding=0) uniform DynamicUBO
{
    mat4 model;
}dubo;

// model position of vertex
layout(location = 0) in vec3 inPosition;

void main() 
{
    // apply camera transform than model than transfrom for outline
    // this order gives better results IMHO, but outline can be putted in different places
    gl_Position =  ubo.proj * ubo.view  * dubo.model * scaleMat(outline.scale) * vec4(inPosition, 1.0);
}