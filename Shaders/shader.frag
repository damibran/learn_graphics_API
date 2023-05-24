#version 450
// simple white light simulation with light from up to down
const vec3 light_dir = vec3(0,0,-1);
const vec4 lightColor = vec4(1.0,1.0,1.0,1.0);

// shader effect data
layout(set=1,binding=0) uniform UnLitTexturedUBO
{
    // gamma corrention
    float gamma;
}ult;

// model diffuse texture
layout(set=2, binding = 0) uniform sampler2D texSampler;

// material simple properties
layout(set=2, binding = 1) uniform Properties
{
    vec4 base_color;
}properties;

// texture coord from vertex shader
layout(location = 0) in vec2 fragTexCoord;
// transformed vertex normal
layout(location = 1) in vec3 inNormal;

// output color for this pixel
layout(location = 0) out vec4 outColor;

void main() 
{
    // phong shading https://en.wikipedia.org/wiki/Phong_shading
    // full ambient strength
    const float ambientStrength = 1.0;
    vec4 ambient = ambientStrength * lightColor;

    // calculate diffuse reflection as dot between nomal and direction *towards* light
    float diff = 1.0*max(dot(inNormal,-light_dir),0.0);
    vec4 diffuse = diff * lightColor;

    // collect all with average balance between ambient and diffuse
    outColor = (ambient + diffuse)*0.5*properties.base_color * texture(texSampler, fragTexCoord);
    
    // add gamma correction to color
    const float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0/gamma));
}